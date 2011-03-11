#include <U2Core/Log.h>
#include <U2Core/Counter.h>

#include "RemoteServiceMachine.h"
#include "WebTransportProtocol.h"
#include "RemoteServiceUtilTasks.h"

#define REMOTE_SERVICE_TASKS_LOG "remote-service-tasks"


namespace U2 {

GetUserTasksInfoTask::GetUserTasksInfoTask( RemoteServiceMachine* m )
 : Task( tr( "GetUserTasksInfo" ), TaskFlags( TaskFlags_FOSCOE ) ), machine(m) 
{
    
    GCOUNTER(cvar,tvar,"GetUserTasksInfo");
    properties.insert(UctpElements::TASK_STATE, QString());
    properties.insert(UctpElements::DATE_SUBMITTED, QString());
    properties.insert(UctpElements::TASK_RESULTS, QString());
}


void GetUserTasksInfoTask::run() {
    assert(machine != NULL);
    if (isCanceled() || hasErrors()) {
        return;
    }

    rsLog.details(tr("Retrieving remote tasks list for: %1").arg(machine->getSettings()->getName()));

    machine->initSession(stateInfo);
    if (hasErrors()) {
        return;
    }
    
    QList<qint64> runningTaskIds;
    runningTaskIds = machine->getActiveTasks(stateInfo);
    if (hasErrors()) {
        return;
    } 
    rsLog.details(tr("Found %1 active remote tasks").arg(runningTaskIds.size()));
    
    QList<qint64> finishedTaskIds;
    finishedTaskIds = machine->getFinishedTasks(stateInfo);
    if (hasErrors()) {
        return;
    } 

    rsLog.details(tr("Found %1 finished remote tasks").arg(finishedTaskIds.size()));
    QList<qint64> taskIds;
    taskIds << runningTaskIds << finishedTaskIds;
    
    foreach(qint64 taskId, taskIds) {
        machine->getTaskProperties(stateInfo, taskId, properties);
        if (hasErrors()) {
            break;
        }

        RemoteTaskInfo info;
        info.taskId = QString("%1").arg(taskId);
        info.date = properties.value(UctpElements::DATE_SUBMITTED);
        info.taskState = properties.value(UctpElements::TASK_STATE);
        info.result = properties.value(UctpElements::TASK_RESULTS);                 
        infoList.append(info);
    }
}


FetchRemoteTaskResultTask::FetchRemoteTaskResultTask( RemoteServiceMachine* m, const QStringList& urls, qint64 id )
: Task( tr( "FetchRemoteTaskResult" ), TaskFlags( TaskFlags_FOSCOE ) ), machine(m), resultUrls(urls), taskId(id) 
{
}

void FetchRemoteTaskResultTask::run() {
    assert(machine != NULL);
    if (isCanceled() || hasErrors()) {
        return;
    }

    machine->getTaskResult(stateInfo, taskId, resultUrls, "out/");
}


DeleteRemoteDataTask::DeleteRemoteDataTask( RemoteServiceMachine* m, qint64 id )
: Task("DeleteRemoteDataTask", TaskFlags_FOSCOE), machine(m), taskId(id)
{
   
}

void DeleteRemoteDataTask::run() {
    assert(machine != NULL);
    if (isCanceled() || hasErrors()) {
        return;
    }

    machine->deleteRemoteTask(stateInfo, taskId);
}


} // namespace U2

