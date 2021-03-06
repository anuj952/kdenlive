/*
Copyright (C) 2012  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>
This file is part of kdenlive. See www.kdenlive.org.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "audioStreamInfo.h"

#include "kdenlive_debug.h"
#include <KLocalizedString>
#include <QString>
#include <cstdlib>

AudioStreamInfo::AudioStreamInfo(const std::shared_ptr<Mlt::Producer> &producer, int audioStreamIndex)
    : m_audioStreamIndex(audioStreamIndex)
    , m_ffmpegAudioIndex(0)
    , m_samplingRate(48000)
    , m_channels(2)
    , m_bitRate(0)
{
    // Fetch audio streams
    int streams = producer->get_int("meta.media.nb_streams");
    for (int ix = 0; ix < streams; ix++) {
        char property[200];
        snprintf(property, sizeof(property), "meta.media.%d.stream.type", ix);
        QString type = producer->get(property);
        if (type == QLatin1String("audio")) {
            m_audioStreams << ix;
        }
    }
    if (audioStreamIndex > -1) {
        QByteArray key;
        key = QStringLiteral("meta.media.%1.codec.sample_fmt").arg(audioStreamIndex).toLocal8Bit();
        m_samplingFormat = QString::fromLatin1(producer->get(key.data()));

        key = QStringLiteral("meta.media.%1.codec.sample_rate").arg(audioStreamIndex).toLocal8Bit();
        m_samplingRate = producer->get_int(key.data());

        key = QStringLiteral("meta.media.%1.codec.bit_rate").arg(audioStreamIndex).toLocal8Bit();
        m_bitRate = producer->get_int(key.data());

        key = QStringLiteral("meta.media.%1.codec.channels").arg(audioStreamIndex).toLocal8Bit();
        m_channels = producer->get_int(key.data());

        setAudioIndex(producer, m_audioStreamIndex);
    }
}

AudioStreamInfo::~AudioStreamInfo() = default;

int AudioStreamInfo::samplingRate() const
{
    return m_samplingRate;
}

int AudioStreamInfo::channels() const
{
    return m_channels;
}

QList <int> AudioStreamInfo::streams() const
{
    return m_audioStreams;
}

int AudioStreamInfo::bitrate() const
{
    return m_bitRate;
}

int AudioStreamInfo::audio_index() const
{
    return m_audioStreamIndex;
}

int AudioStreamInfo::ffmpeg_audio_index() const
{
    return m_ffmpegAudioIndex;
}

void AudioStreamInfo::dumpInfo() const
{
    qCDebug(KDENLIVE_LOG) << "Info for audio stream " << m_audioStreamIndex << "\n\tChannels: " << m_channels << "\n\tSampling rate: " << m_samplingRate
                          << "\n\tBit rate: " << m_bitRate;
}

void AudioStreamInfo::setAudioIndex(const std::shared_ptr<Mlt::Producer> &producer, int ix)
{
    m_audioStreamIndex = ix;
    if (ix > -1) {
        int streams = producer->get_int("meta.media.nb_streams");
        QList<int> audioStreams;
        for (int i = 0; i < streams; ++i) {
            QByteArray propertyName = QStringLiteral("meta.media.%1.stream.type").arg(i).toLocal8Bit();
            QString type = producer->get(propertyName.data());
            if (type == QLatin1String("audio")) {
                audioStreams << i;
            }
        }
        if (audioStreams.count() > 1 && m_audioStreamIndex < audioStreams.count()) {
            m_ffmpegAudioIndex = audioStreams.indexOf(m_audioStreamIndex);
        }
    }
}

QMap<int, QString> AudioStreamInfo::streamInfo(Mlt::Properties sourceProperties)
{
    QMap<int, QString> streamInfo;
    char property[200];
    for (int ix : m_audioStreams) {
        memset(property, 0, 200);
        snprintf(property, sizeof(property), "meta.media.%d.codec.channels", ix);
        int chan = sourceProperties.get_int(property);
        QString channelDescription;
        switch (chan) {
            case 1:
                channelDescription = i18n("Mono ");
                break;
            case 2:
                channelDescription = i18n("Stereo ");
                break;
            default:
                channelDescription = i18n("%1 channels ", chan);
                break;
        }
        // Frequency
        memset(property, 0, 200);
        snprintf(property, sizeof(property), "meta.media.%d.codec.sample_rate", ix);
        QString frequency(sourceProperties.get(property));
        if (frequency.endsWith(QLatin1String("000"))) {
            frequency.chop(3);
            frequency.append(i18n("kHz "));
        } else {
            frequency.append(i18n("Hz "));
        }
        channelDescription.append(frequency);
        memset(property, 0, 200);
        snprintf(property, sizeof(property), "meta.media.%d.codec.name", ix);
        channelDescription.append(sourceProperties.get(property));
        streamInfo.insert(ix, channelDescription);
    }
    return streamInfo;
}
