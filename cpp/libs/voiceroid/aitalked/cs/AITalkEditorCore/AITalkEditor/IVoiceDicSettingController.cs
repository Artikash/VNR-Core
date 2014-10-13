namespace AITalkEditor
{
    using System;
    using System.Collections.Generic;

    public interface IVoiceDicSettingController
    {
        bool CheckIgnoredVoiceName(string voiceName, List<string> ignoredVoiceNames);
        List<VoiceInfo> EnumVoiceNames(string dbsPath, int voiceSamplePerSec);
        void UpdateVoiceDicSetting(VoiceDicSettingArgs args);
    }
}

