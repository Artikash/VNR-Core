namespace AITalkEditor
{
    using System;
    using System.Drawing;

    public interface ITextFormatSettingController
    {
        void UpdateFontSetting(Color foreColor, Font font);
        void UpdateWordWrapSetting(bool wordWrap);
    }
}

