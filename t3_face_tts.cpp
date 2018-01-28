#include "t3_face_tts.h"

T3_Face_TTS::T3_Face_TTS()
{

    _soundEffect = new QSoundEffect(this);
    _player = new QMediaPlayer();
    connect(_player,&QMediaPlayer::stateChanged,this,&T3_Face_TTS::playerStateChange);
    initTTS();
}

int T3_Face_TTS::initTTS()
{
    const char* login_params         = "appid = 5a5ef3cf, work_dir = .";
    const char* login_name           = "31011315@qq.com";
    const char* login_pwd            = "a000000A";
    ret = MSPLogin(login_name, login_pwd, login_params); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
        if (MSP_SUCCESS != ret)
        {
            printf("MSPLogin failed, error code: %d.\n", ret);
            return 0 ;//登录失败，退出登录
        }
    _sessionBeginParams = "engine_type = local,voice_name=xiaoyan, text_encoding = UTF8, tts_res_path = fo|res/tts/xiaoyan.jet;fo|res/tts/common.jet, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
    _filename           = "tts.wav"; //合成的语音文件名称
}

int T3_Face_TTS::textToSpeech(const char *src_text)
{
    int          ret          = -1;
    FILE*        fp           = NULL;
    const char*  sessionID    = NULL;
    unsigned int audio_len    = 0;
    wave_pcm_hdr wav_hdr      = default_wav_hdr;
    int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

    if (NULL == src_text || NULL == _filename)
    {
        printf("params is error!\n");
        return ret;
    }
    fp = fopen(_filename, "wb");
    if (NULL == fp)
    {
        printf("open %s error.\n", _filename);
        return ret;
    }
    /* 开始合成 */
    sessionID = QTTSSessionBegin(_sessionBeginParams, &ret);
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSSessionBegin failed, error code: %d.\n", ret);
        fclose(fp);
        return ret;
    }
    ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSTextPut failed, error code: %d.\n",ret);
        QTTSSessionEnd(sessionID, "TextPutError");
        fclose(fp);
        return ret;
    }
    printf("正在合成 ...\n");
    fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
    while (1)
    {
        /* 获取合成音频 */
        const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
        if (MSP_SUCCESS != ret)
            break;
        if (NULL != data)
        {
            fwrite(data, audio_len, 1, fp);
            wav_hdr.data_size += audio_len; //计算data_size大小
        }
        if (MSP_TTS_FLAG_DATA_END == synth_status)
            break;
    }
    printf("\n");
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSAudioGet failed, error code: %d.\n",ret);
        QTTSSessionEnd(sessionID, "AudioGetError");
        fclose(fp);
        return ret;
    }
    /* 修正wav文件头数据的大小 */
    wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);

    /* 将修正过的数据写回文件头部,音频文件为wav格式 */
    fseek(fp, 4, 0);
    fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
    fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
    fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
    fclose(fp);
    fp = NULL;
    /* 合成完毕 */
    ret = QTTSSessionEnd(sessionID, "Normal");
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSSessionEnd failed, error code: %d.\n",ret);
    }
    _player->setMedia(QUrl::fromLocalFile("tts.wav"));
    _player->setVolume(30);
    _player->play();
    return ret;
}

int T3_Face_TTS::inputToText(QString role, int gender, QString name)
{
    QSqlQuery query_;
    query_.prepare("select voice from T3FaceUserType where UserType = ?");
    query_.bindValue(0,role);
    query_.exec();
    query_.next();
    QString text = query_.value(0).toString();
    text.replace("（姓名）",name);
    switch (gender) {
    case 0:
        text.replace("（性别）","先生");
        break;
    case 1:
        text.replace("（性别）","女士");
        break;
    default:
        text.replace("（性别）","同学");
        break;
    }
    T3LOG << text;
    _textList.append(text);

}
int T3_Face_TTS::textListToSpeech()
{
    if(_textListIndex < _textList.size())
    {
        QByteArray temp_ = _textList[_textListIndex].toLatin1();
        const char * text_= (const char * ) temp_.data();
        textToSpeech(text_);
        _textListIndex++;
    }else
    {
        _textListIndex = 0 ;
        _textList.clear();
        T3LOG << "sound play end";
    }
}

void T3_Face_TTS::playerStateChange(QMediaPlayer::State newState)
{
    if(QMediaPlayer::StoppedState == newState)
    {
        textListToSpeech();
    }
}