/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2020 Bernhard Trinnes <bernhard.trinnes@nymea.io>        *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef YEELIGHT_H
#define YEELIGHT_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QTcpSocket>

#include "network/networkaccessmanager.h"
#include "devices/device.h"

#include <QRgb>

class Yeelight : public QObject
{
    Q_OBJECT
public:
    enum Property {
        Power,      //on: smart LED is turned on / off: smart LED is turned off
        Bright,     //Brightness percentage. Range 1 ~ 100
        Ct,         //Color temperature. Range 1700 ~ 6500(k)
        Rgb,        //Color. Range 1 ~ 16777215
        Hue,        //Hue. Range 0 ~ 359
        Sat,        //Saturation. Range 0 ~ 100
        ColorMode,  //1: rgb mode / 2: color temperature mode / 3: hsv mode
        Flowing,    //0: no flow is running / 1:color flow is running
        DelayOff,   //The remaining time of a sleep timer. Range 1 ~ 60 (minutes)
        FlowParams, //Current flow parameters (only meaningful when 'flowing' is 1)
        MusicOn,    //1: Music mode is on / 0: Music mode is off
        Name,       //The name of the device set by “set_name” command
        BgPower,    //Background light power status
        BgFlowing,  //Background light is flowing
        BgFlowParams, //Current flow parameters of background light
        BgCt,       //Color temperature of background light
        BgLmode,    //1: rgb mode / 2: color temperature mode / 3: hsv mode
        BgBright,   //Brightness percentage of background light
        BgRgb,      //Color of background light
        BgHue,      //Hue of background light
        BgSat,      //Saturation of background light
        NlBr,       //Brightness of night mode light
        ActiveMode  //0: daylight mode / 1: moonlight mode (ceiling light only)
    };

    explicit Yeelight(NetworkAccessManager *networkManager, const QHostAddress &address,  quint16 port = 55443, QObject *parent = nullptr);
    bool connected();

    int getParam(QList<Property> properties);
    int setName(const QString &name);
    int setColorTemperature(int mirad, int msFadeTime=500);
    int setRgb(QRgb color, int msFadeTime = 500);
    int setBrightness(int percentage, int msFadeTime = 500);
    int setPower(bool power, int msFadeTime = 500);
    int setDefault();


private:

    QTcpSocket *m_socket = nullptr;
    QHostAddress m_address;
    qint16 m_port;
    NetworkAccessManager *m_networkManager = nullptr;

private slots:
    void onRefreshTimeout();
    void onStateChanged(QAbstractSocket::SocketState state);

signals:
    void connectionChanged(bool connected);
    void authenticationStatusChanged(bool authenticated);
    void requestExecuted(int requestId, bool success);
    void propertyReceived(Property property, QVariant value);
};
#endif // YEELIGHT_H