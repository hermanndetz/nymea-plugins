/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "integrationpluginremotessh.h"
#include "plugininfo.h"

#include <QFile>
#include <QDir>

IntegrationPluginRemoteSsh::IntegrationPluginRemoteSsh()
{

}

void IntegrationPluginRemoteSsh::startPairing(ThingPairingInfo *info)
{
    if (info->thingClassId() == reverseSshThingClassId) {
        info->finish(Thing::ThingErrorNoError, "Please enter user name and password");
    } else {
        qCWarning(dcRemoteSsh()) << "Unhandled pairing method!";
        info->finish(Thing::ThingErrorCreationMethodNotSupported);
    }
}

void IntegrationPluginRemoteSsh::confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret)
{
    if (info->thingClassId() == reverseSshThingClassId) {

            pluginStorage()->beginGroup(info->thingId().toString());
            pluginStorage()->setValue("username", username);
            pluginStorage()->setValue("password", secret);
            pluginStorage()->endGroup();

    } else {
        qCWarning(dcRemoteSsh()) << "Invalid thingClassId -> no pairing possible with this device";
        info->finish(Thing::ThingErrorThingClassNotFound);
    }
}


void IntegrationPluginRemoteSsh::setupThing(ThingSetupInfo *info)
{
    Thing *thing = info->thing();

    qCDebug(dcRemoteSsh()) << "Setup" << thing->name() << thing->params();
    if (thing->thingClassId() == reverseSshThingClassId) {

        return info->finish(Thing::ThingErrorNoError);
    } else  if (thing->thingClassId() == tmateThingClassId) {

        return info->finish(Thing::ThingErrorNoError);
    } else {
        Q_ASSERT_X(false, "setupThing", QString("Unhandled thingClassId: %1").arg(thing->thingClassId().toString()).toUtf8());
    }
}

void IntegrationPluginRemoteSsh::postSetupThing(Thing *thing)
{
    Q_UNUSED(thing)
    if (!m_pluginTimer) {
        m_pluginTimer = hardwareManager()->pluginTimerManager()->registerTimer(10);
        connect(m_pluginTimer, &PluginTimer::timeout, this, [this] {

            foreach(QProcess *process, m_reverseSSHProcess.keys()) {
                if (process->state() == QProcess::NotRunning)
                    qCDebug(dcRemoteSsh()) << "SSH Process not running";
            }

            foreach(QProcess *process, m_tmateProcess.keys()) {
                if (process->state() == QProcess::NotRunning)
                    qCDebug(dcRemoteSsh()) << "tmate Process not running";
            }
        });
    }
}

void IntegrationPluginRemoteSsh::executeAction(ThingActionInfo *info)
{
    Thing *thing = info->thing();
    Action action = info->action();
    if (thing->thingClassId() == reverseSshThingClassId ) {

        if (action.actionTypeId() == reverseSshConnectedActionTypeId) {

            if (action.param(reverseSshConnectedActionConnectedParamTypeId).value().toBool() == true) {
                QProcess *process = startReverseSSHProcess(thing);
                m_reverseSSHProcess.insert(process, thing);
                m_startingProcess.insert(process, info);
                // in case action call is cancelled, detach result reporting
                connect(info, &ThingActionInfo::destroyed, process, [this, process]{
                    m_startingProcess.remove(process);
                });
                return;
            } else {
                QProcess *process =  m_reverseSSHProcess.key(thing);

                // Check if the application is running...
                if (!process)
                    return info->finish(Thing::ThingErrorNoError);

                if (process->state() == QProcess::NotRunning)
                    return info->finish(Thing::ThingErrorNoError);

                process->kill();
                m_killingProcess.insert(process, info);
                // in case action call is cancelled, detach result reporting
                connect(info, &ThingActionInfo::destroyed, process, [this, process]{
                    m_killingProcess.remove(process);
                });
                return;
            }
        } else {
            Q_ASSERT_X(false, "executeAction", QString("Unhandled actionTypeId: %1").arg(action.actionTypeId().toString()).toUtf8());
        }
    } else if (thing->thingClassId() == tmateThingClassId ) {
        if (action.actionTypeId() == tmateConnectedActionTypeId) {
            if (action.param(tmateConnectedActionConnectedParamTypeId).value().toBool() == true) {
                QProcess *process = startTmateProcess(thing);
                m_tmateProcess.insert(process, thing);
                m_startingProcess.insert(process, info);
                // in case action call is cancelled, detach result reporting
                connect(info, &ThingActionInfo::destroyed, process, [this, process]{
                    m_startingProcess.remove(process);
                });
                return;
            } else {
                QProcess *process =  m_reverseSSHProcess.key(thing);

                // Check if the application is running...
                if (!process)
                    return info->finish(Thing::ThingErrorNoError);

                if (process->state() == QProcess::NotRunning)
                    return info->finish(Thing::ThingErrorNoError);

                process->kill();
                m_killingProcess.insert(process, info);
                // in case action call is cancelled, detach result reporting
                connect(info, &ThingActionInfo::destroyed, process, [this, process]{
                    m_killingProcess.remove(process);
                });
                return;
            }
        } else {
            Q_ASSERT_X(false, "executeAction", QString("Unhandled actionTypeId: %1").arg(action.actionTypeId().toString()).toUtf8());
        }
    } else {
        Q_ASSERT_X(false, "executeAction", QString("Unhandled thingClassId: %1").arg(thing->thingClassId().toString()).toUtf8());
    }
}


void IntegrationPluginRemoteSsh::thingRemoved(Thing *thing)
{
    if (myThings().empty()) {
        hardwareManager()->pluginTimerManager()->unregisterTimer(m_pluginTimer);
        m_pluginTimer = nullptr;
    }

    if (thing->thingClassId() == reverseSshThingClassId) {
        QProcess *process =  m_reverseSSHProcess.key(thing);
        if (!process)
            return;

        m_reverseSSHProcess.remove(process);
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
        process->deleteLater();
    } else if (thing->thingClassId() == tmateThingClassId ) {
        QProcess *process =  m_tmateProcess.key(thing);
        if (!process)
            return;

        m_tmateProcess.remove(process);
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
        process->deleteLater();
    } else {
        Q_ASSERT_X(false, "thingRemoved", QString("Unhandled thingClassId: %1").arg(thing->thingClassId().toString()).toUtf8());
    }
}

QProcess * IntegrationPluginRemoteSsh::startReverseSSHProcess(Thing *thing)
{
    qCDebug(dcRemoteSsh()) << "Start reverse SSH";
    QProcess *process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [process, thing, this](int exitCode, QProcess::ExitStatus exitStatus){

        if(exitStatus != QProcess::NormalExit || exitCode != 0) {
            qCWarning(dcRemoteSsh()) << "Error:" << process->readAllStandardError();
        }

        if (m_reverseSSHProcess.contains(process)) {
            qCDebug(dcRemoteSsh()) << "SSH process finished";
            Thing *thing = m_reverseSSHProcess.value(process);
            thing->setStateValue(reverseSshConnectedStateTypeId, false);
            m_reverseSSHProcess.remove(process);
        }
    });
    connect(process, &QProcess::stateChanged, this, [process, thing, this] (QProcess::ProcessState state) {

        switch (state) {
        case QProcess::Running:
            thing->setStateValue(reverseSshConnectedStateTypeId, true);
            if (m_startingProcess.contains(process)) {
                m_startingProcess.take(process)->finish(Thing::ThingErrorNoError);
            }
            break;

        case QProcess::NotRunning:
            if (thing)
                thing->setStateValue(reverseSshConnectedStateTypeId, false);

            if (m_startingProcess.contains(process)) {
                m_startingProcess.take(process)->finish(Thing::ThingErrorInvalidParameter);
            }

            if (m_killingProcess.contains(process)) {
                m_killingProcess.take(process)->finish(Thing::ThingErrorNoError);
                m_reverseSSHProcess.remove(process);
            }
            break;
        default:
            break;
        }

    });
    connect(process, &QProcess::readyRead, this, [process, this] {
        QByteArray data = process->readAll();
        qCWarning(dcRemoteSsh()) << "process read" << data;
    });

    QStringList arguments;
    int localPort    = thing->paramValue(reverseSshThingLocalPortParamTypeId).toInt();
    int remotePort   = thing->paramValue(reverseSshThingRemotePortParamTypeId).toInt();
    QString address  = thing->paramValue(reverseSshThingAddressParamTypeId).toString();

    pluginStorage()->beginGroup(thing->id().toString());
    QString user = pluginStorage()->value("username").toString();
    QString password = pluginStorage()->value("password").toString();
    pluginStorage()->endGroup();

    arguments << "-p" << password;
    arguments << "ssh" << "-o StrictHostKeyChecking=no" << "-oUserKnownHostsFile=/dev/null";
    arguments << "-TN" << "-R" << QString("%1:localhost:%2").arg(remotePort).arg(localPort) << QString("%1@%2").arg(user, address);
    process->start(QStringLiteral("sshpass"), arguments);
    qCDebug(dcRemoteSsh()) << "Command:" << process->program() << process->arguments();
    return process;
}

QProcess *IntegrationPluginRemoteSsh::startTmateProcess(Thing *thing)
{
    qCDebug(dcRemoteSsh()) << "Start tmate";
    QProcess *process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [process, thing, this](int exitCode, QProcess::ExitStatus exitStatus){

        if(exitStatus != QProcess::NormalExit || exitCode != 0) {
            qCWarning(dcRemoteSsh()) << "Error:" << process->readAllStandardError();
        }
        qCDebug(dcRemoteSsh()) << "tmate process finished";
        thing->setStateValue(tmateConnectedStateTypeId, false);

    });
    connect(process, &QProcess::stateChanged, this, [process, thing, this] (QProcess::ProcessState state) {

        switch (state) {
        case QProcess::Running:
            thing->setStateValue(tmateConnectedStateTypeId, true);
            if (m_startingProcess.contains(process)) {
                m_startingProcess.take(process)->finish(Thing::ThingErrorNoError);
            }
            break;

        case QProcess::NotRunning:
            if (thing)
                thing->setStateValue(tmateConnectedStateTypeId, false);

            if (m_startingProcess.contains(process)) {
                m_startingProcess.take(process)->finish(Thing::ThingErrorInvalidParameter);
            }

            if (m_killingProcess.contains(process)) {
                m_killingProcess.take(process)->finish(Thing::ThingErrorNoError);
                m_reverseSSHProcess.remove(process);
            }
            break;
        default:
            break;
        }

    });
    connect(process, &QProcess::readyRead, this, [process, this] {
        QByteArray data = process->readAll();
        qCWarning(dcRemoteSsh()) << "process read" << data;
    });

    process->start(QStringLiteral("tmate -F"));
    qCDebug(dcRemoteSsh()) << "Command:" << process->program() << process->arguments();
    return process;
}
