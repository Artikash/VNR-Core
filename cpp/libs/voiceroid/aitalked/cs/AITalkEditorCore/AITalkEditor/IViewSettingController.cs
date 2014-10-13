namespace AITalkEditor
{
    using System;

    public interface IViewSettingController
    {
        void UpdateSettingsPaneVisible(bool settingsPaneVisible);
        void UpdateTuningPaneVisible(bool tuningPaneVisible);
    }
}

