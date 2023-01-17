#include <Processors/Transforms/RemoteDependencyTransform.h>

#include <QueryPipeline/RemoteQueryExecutor.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int LOGICAL_ERROR;
}

ReadFromMergeTreeDependencyTransform::ReadFromMergeTreeDependencyTransform(const Block & header)
    : DependentProcessor(InputPorts(1, header), OutputPorts(1, header))
    , data_port(&inputs.front())
{
}

void ReadFromMergeTreeDependencyTransform::connectToScheduler(ResizeProcessor & scheduler)
{
    inputs.emplace_back(Block{}, this);
    dependency_port = &inputs.back();
    auto * free_port = scheduler.getFreeOutputPortIfAny();
    if (!free_port)
        throw Exception(ErrorCodes::LOGICAL_ERROR, "There are no free input ports in scheduler. This is a bug");

    connect(*free_port, *dependency_port);
}

IProcessor::Status ReadFromMergeTreeDependencyTransform::prepare()
{
    Status status = Status::Ready;

    while (status == Status::Ready)
    {
        status = !has_data ? prepareConsume()
                           : prepareGenerate();
    }

    return status;
}

IProcessor::Status ReadFromMergeTreeDependencyTransform::prepareConsume()
{
    auto & output_port = getOutputPort();

    /// Check all outputs are finished or ready to get data.
    if (output_port.isFinished())
    {
        data_port->close();
        dependency_port->close();
        return Status::Finished;
    }

    /// Try get chunk from input.
    if (data_port->isFinished())
    {
        dependency_port->close();
        output_port.finish();
        return Status::Finished;
    }

    if (!dependency_port->isFinished())
    {
        dependency_port->setNeeded();
        if (!dependency_port->hasData())
            return Status::NeedData;
    }

    data_port->setNeeded();
    if (!data_port->hasData())
        return Status::NeedData;

    if (!dependency_port->isFinished())
        dependency_port->pull();

    chunk = data_port->pull();
    has_data = true;

    return Status::Ready;
}

IProcessor::Status ReadFromMergeTreeDependencyTransform::prepareGenerate()
{
    auto & output_port = getOutputPort();
    if (!output_port.isFinished() && output_port.canPush())
    {
        output_port.push(std::move(chunk));
        has_data = false;
        return Status::Ready;
    }

    return Status::PortFull;
}

}
