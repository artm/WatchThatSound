#pragma once

#include <QString>

namespace WTS {

class Exception {
    QString m_message;

public:
    Exception(const QString& message)
        : m_message(message)
    { }

    const QString& message() const { return m_message; }
};

}
