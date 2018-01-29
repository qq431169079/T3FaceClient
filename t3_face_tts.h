#ifndef T3_FACE_TTS_H
#define T3_FACE_TTS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QSoundEffect>
#include <QUrl>
#include "t3_log.h"
extern "C"
{
#include "msp_cmn.h"
#include "msp_errors.h"
#include "msp_types.h"
#include "qtts.h"
}
typedef int SR_DWORD;
typedef short int SR_WORD ;
/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
    char            riff[4];                // = "RIFF"
    int				size_8;                 // = FileSize - 8
    char            wave[4];                // = "WAVE"
    char            fmt[4];                 // = "fmt "
    int				fmt_size;				// = 下一个结构体的大小 : 16

    short int       format_tag;             // = PCM : 1
    short int       channels;               // = 通道数 : 1
    int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
    int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
    short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
    short int       bits_per_sample;        // = 量化比特数: 8 | 16

    char            data[4];                // = "data";
    int				data_size;              // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

class T3_Face_TTS : public QObject
{
    Q_OBJECT
public:
    static T3_Face_TTS *getTTS();
    /**
     * @brief inputToText 根据不同的输入合成相应的文字
     * @param role　用户的类型　没有则用“”来表示
     * @param gender　用户的性别　男：０，女：１，无法识别：－１
     * @param name　用户的名字　没有则使用“”表示
     * @return
     */
    int inputToText(QString role,int gender,QString name);


private:
    T3_Face_TTS();
    int initTTS();
    int textToSpeech(const char* src_text);
    int textListToSpeech();
    int ret = MSP_SUCCESS;
    const char* _sessionBeginParams;
    const char* _filename;
    bool _isPlayVoice = false;
    QList<QString> _textList;
    int _textListIndex = 0;
    QSoundEffect *_soundEffect;


    /* 默认wav音频头部数据 */
    wave_pcm_hdr default_wav_hdr =
    {
        { 'R', 'I', 'F', 'F' },
        0,
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16,
        1,
        1,
        16000,
        32000,
        2,
        16,
        {'d', 'a', 't', 'a'},
        0
    };

private slots:
    void playingChange();

};

#endif // T3_FACE_TTS_H
