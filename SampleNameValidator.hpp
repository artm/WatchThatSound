#ifndef SAMPLENAMEVALIDATOR_HPP
#define SAMPLENAMEVALIDATOR_HPP

#include <QValidator>

namespace WTS {

class Project;
class WaveformWidget;

class SampleNameValidator : public QValidator
{
    Q_OBJECT
public:
    explicit SampleNameValidator(Project * project, WaveformWidget * waveform);

protected:
    State validate ( QString & input, int & pos ) const;

    Project * m_project;
    WaveformWidget * m_waveform;
};

}

#endif // SAMPLENAMEVALIDATOR_HPP
