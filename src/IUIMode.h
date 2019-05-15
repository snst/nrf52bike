#ifndef IUIMODE_H_
#define IUIMODE_H_

class IUIMode
{
    public:
      
    enum eUiMode_t
    {
        eStartup,
        eCsc,
        eKomoot,
        eSettings
    };
    virtual void SetUiMode(IUIMode::eUiMode_t mode) = 0;
    virtual void SetUiBrightness(uint8_t val) = 0;
};

#endif