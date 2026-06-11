#include "soundout.h"

#include <QDateTime>
#include <QAudioDevice>
#include <QAudioSink>
#include <QSysInfo>
#include <qmath.h>
#include <QDebug>

#include "Logger.hpp"
#include "Audio/AudioDevice.hpp"

#include "moc_soundout.cpp"

bool SoundOutput::checkStream () const
{
  bool result {false};

  Q_ASSERT_X (m_stream, "SoundOutput", "programming error");
  if (m_stream) {
    switch (m_stream->error ())
      {
      case QAudio::OpenError:
        Q_EMIT error (tr ("An error opening the audio output device has occurred."));
        break;

      case QAudio::IOError:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        Q_EMIT error (tr ("An error occurred during write to the audio output device."));
#endif
        break;

      case QAudio::UnderrunError:
        Q_EMIT error (tr ("Audio data not being fed to the audio output device fast enough."));
        break;

      case QAudio::FatalError:
        Q_EMIT error (tr ("Non-recoverable error, audio output device not usable at this time."));
        break;

      case QAudio::NoError:
        result = true;
        break;
      }
  }
  return result;
}

void SoundOutput::setFormat (QAudioDevice const& device, unsigned channels, int frames_buffered)
{
  Q_ASSERT (0 < channels && channels < 3);
  m_device = device;
  m_channels = channels;
  m_framesBuffered = frames_buffered;
}

void SoundOutput::restart (QIODevice * source)
{
  if (!m_device.isNull ())
    {
      QAudioFormat format (m_device.preferredFormat ());
      format.setChannelCount (m_channels);
      format.setSampleRate (48000);
      format.setSampleFormat (QAudioFormat::Int16);
      if (!format.isValid ())
        {
          Q_EMIT error (tr ("Requested output audio format is not valid."));
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
      else if (!m_device.isFormatSupported (format))
        {
          Q_EMIT error (tr ("Requested output audio format is not supported on device."));
        }
#endif
      else
        {
          m_stream.reset (new QAudioSink (m_device, format));
          qDebug () << "SoundOutput::restart Selected audio output format:" << m_stream -> format();
	  checkStream ();
          m_stream->setVolume (m_volume);
//          m_stream->setNotifyInterval(1000);
          error_ = false;

          connect (m_stream.data(), &QAudioSink::stateChanged, this, &SoundOutput::handleStateChanged);
//          connect (m_stream.data(), &QAudioSink::notify, [this] () {checkStream ();});

          //      qDebug() << "A" << m_volume << m_stream->notifyInterval();
        }
    }
  if (!m_stream)
    {
      if (!error_)
        {
          error_ = true;        // only signal error once
          Q_EMIT error (tr ("No audio output device configured."));
        }
      return;
    }
  else
    {
      error_ = false;
    }

  // we have to set this before every start on the stream because the
  // Windows implementation seems to forget the buffer size after a
  // stop.
  // qDebug () << "SoundOut default buffer size (bytes):" << m_stream->bufferSize () << "m_framesBuffered: " << m_framesBuffered;
  if (m_framesBuffered > 0)
    {
      m_stream->setBufferSize (m_stream->format().bytesForFrames (m_framesBuffered));
    }
  m_stream->start (source);
//  LOG_DEBUG ("Bytes avail (bytes): " << source->bytesAvailable ());
//  LOG_DEBUG ("Buffer size (bytes): " << m_stream->bufferSize ());
}

void SoundOutput::suspend ()
{
  if (m_stream && QAudio::ActiveState == m_stream->state ())
    {
      m_stream->suspend ();
      checkStream ();
    }
}

void SoundOutput::resume ()
{
  if (m_stream && QAudio::SuspendedState == m_stream->state ())
    {
      m_stream->resume ();
      checkStream ();
    }
}

void SoundOutput::reset ()
{
  if (m_stream)
    {
      m_stream->reset ();
      checkStream ();
    }
}

void SoundOutput::stop ()
{
  if (m_stream)
    {
      m_stream->reset ();
      m_stream->stop ();
    }
#ifdef __APPLE__
  // this code is here to help certain rigs not drop audio, however on Sequoia this causes audio to drop, so we don't do it!
  if( QOperatingSystemVersion::current() < QOperatingSystemVersion(QOperatingSystemVersion::MacOS, 15) )
    m_stream.reset ();
#endif
}

qreal SoundOutput::attenuation () const
{
  return -(20. * qLn (m_volume) / qLn (10.));
}

void SoundOutput::setAttenuation (qreal a)
{
  Q_ASSERT (0. <= a && a <= 999.);
  m_volume = qPow(10.0, -a/20.0);
  // qDebug () << "SoundOut: attn = " << a << ", vol = " << m_volume;
  if (m_stream)
    {
      m_stream->setVolume (m_volume);
    }
}

void SoundOutput::resetAttenuation ()
{
  m_volume = 1.;
  if (m_stream)
    {
      m_stream->setVolume (m_volume);
    }
}

void SoundOutput::handleStateChanged (QAudio::State newState)
{
  switch (newState)
    {
    case QAudio::IdleState:
      Q_EMIT status (tr ("Idle"));
      break;

    case QAudio::ActiveState:
      Q_EMIT status (tr ("Sending"));
      break;

    case QAudio::SuspendedState:
      Q_EMIT status (tr ("Suspended"));
      break;

//#if QT_VERSION >= QT_VERSION_CHECK (5, 10, 0)
//    case QAudio::InterruptedState:
//      Q_EMIT status (tr ("Interrupted"));
//      break;
//#endif

    case QAudio::StoppedState:
      if (!checkStream ())
        {
          Q_EMIT status (tr ("Error"));
        }
      else
        {
          Q_EMIT status (tr ("Stopped"));
        }
      break;
    }
}
