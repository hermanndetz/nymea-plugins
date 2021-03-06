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

#ifndef WS281XSPICONTROLLER_H
#define WS281XSPICONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QColor>
#include <QTime>

#include "../../rpi_ws281x/ws2811.h"

#define WS281X_MAX_BRIGHTNESS 255
#define WS281X_MIN_BRIGHTNESS 0

class Ws281xSpiController : public QObject
{
    Q_OBJECT

public:
    explicit Ws281xSpiController();
    ~Ws281xSpiController();

    bool setBrightness(int brightness);
    bool setColor(QColor color);
    bool setLength(int length);
    bool setPower(bool power);
    bool setSPIPin(int spiPin);
private:
    int             m_brightness{255};
    QColor          m_color{0xFFFFFF};
    ws2811_t        m_ledstring{};
    int             m_length{8};
    bool            m_power{false};
    int             m_spiPin{10};

    void freeLEDString();
    bool initLEDString();
    bool update();

signals:
    void brightnessChanged(const int brightness);
    void colorChanged(const QColor &color);
    void lengthChanged(const int length);
    void powerChanged(const bool power);
    void spiPinChanged(const int spiPin);
};

#endif // WS281XSPICONTROLLER_H

