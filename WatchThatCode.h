#ifndef WATCH_THAT_CODE_H
#define WATCH_THAT_CODE_H

namespace WTS {

class AssertFailed {
public:
    AssertFailed(const QString& cond, const QString& file, int line,
                 QString extra = QString())
    {
        m_message = QString("%1:%2: assertion '%3' failed")
                .arg(file).arg(line).arg(cond);

        if (extra.size())
            m_message += ". " + extra + ".";
    }
    QString message() const { return m_message; }
    // 'p' for printable/pretty
    const char * pMessage() const { return qPrintable(m_message); }
protected:
    QString m_message;
};

#define TRY_ASSERT(cond) \
    do { if (!(cond)) { throw AssertFailed(#cond, __FILE__, __LINE__); } } while (0)
#define TRY_ASSERT_X(cond, message) \
    do { if (!(cond)) { throw AssertFailed(#cond, __FILE__, __LINE__, message); \
    } } while (0)

}
#endif // WATCH_THAT_CODE_H
