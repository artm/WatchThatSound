#include "SampleNameValidator.hpp"

#include "Project.h"
#include "WaveformWidget.h"

using namespace WTS;

WTS::SampleNameValidator::SampleNameValidator(Project * project, WaveformWidget * waveform) :
    QValidator(project),
    m_project(project),
    m_waveform(waveform)
{
}

QValidator::State WTS::SampleNameValidator::validate(QString &input, int &pos) const
{
    State state = QValidator::Intermediate;

    SoundBuffer * editedBuffer = m_waveform->soundBuffer();
    QString fname = SoundBuffer::makeFileName( input );

    if (!editedBuffer) {
        // if no buffer is being edited, keep it grey no matter what
        state = QValidator::Acceptable;
        goto return_state;
    }

    if (fname == ".wav") goto return_state;

    foreach( WtsAudio::BufferAt * bat, m_project->getSequence() ) {
        if (bat->buffer() != editedBuffer) {
            QString otherFname = SoundBuffer::makeFileName( bat->buffer()->name() );
            if (fname == otherFname)
                goto return_state;
        }
    }
    state = QValidator::Acceptable;

return_state:
    return state;
}
