namespace AITalkEditor
{
    using AITalk;
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;

    public class SoundOutputSettingView : Form
    {
        private ISoundOutputSettingController _controller;
        private Button btnApply;
        private Button btnCancel;
        private Button btnInitialize;
        private Button btnOK;
        private CheckBox chkFileHeader;
        private CheckBox chkSaveText;
        private IContainer components;
        private GroupBox grp;
        private GroupBox grpWaveFormat;
        private RadioButton rad11KHz;
        private RadioButton rad16KHz;
        private RadioButton rad22KHz;
        private RadioButton rad8KHz;
        private RadioButton rad8KHzMuLaw;

        public SoundOutputSettingView(ISoundOutputSettingController c, AppSettings appSettings, UserSettings userSettings)
        {
            this.InitializeComponent();
            this._controller = c;
            this.rad22KHz.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.rad16KHz.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.rad11KHz.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.rad8KHz.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.rad8KHzMuLaw.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.chkFileHeader.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.chkSaveText.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.btnOK.Click += new EventHandler(this.btnOK_Click);
            this.btnCancel.Click += new EventHandler(this.btnCancel_Click);
            this.btnApply.Click += new EventHandler(this.btnApply_Click);
            this.Initialize(appSettings, userSettings);
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            this.UpdateSoundOutputSetting();
            this.btnApply.Enabled = false;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            base.DialogResult = DialogResult.Cancel;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.UpdateSoundOutputSetting();
            base.DialogResult = DialogResult.OK;
        }

        private void Controls_CheckedChanged(object sender, EventArgs e)
        {
            this.btnApply.Enabled = true;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing && (this.components != null))
            {
                this.components.Dispose();
            }
            base.Dispose(disposing);
        }

        public void Initialize(AppSettings appSettings, UserSettings userSettings)
        {
            if (!appSettings.Function.SoundOutputFormatSettingsEnabled)
            {
                this.grpWaveFormat.Visible = false;
                int num = this.grp.Top - this.grpWaveFormat.Top;
                foreach (Control control in base.Controls)
                {
                    if (control.Top > this.grpWaveFormat.Top)
                    {
                        control.Top -= num;
                    }
                }
                base.Height -= num;
            }
            if (userSettings.VoiceSamplePerSec != 0x3e80)
            {
                this.rad16KHz.Enabled = false;
                this.rad8KHz.Enabled = false;
                this.rad8KHzMuLaw.Enabled = false;
                if (userSettings.SoundOutput.SamplePerSec == 0x2b11)
                {
                    this.rad11KHz.Checked = true;
                }
                else
                {
                    this.rad22KHz.Checked = true;
                }
            }
            else
            {
                this.rad22KHz.Enabled = false;
                this.rad11KHz.Enabled = false;
                if (userSettings.SoundOutput.SamplePerSec == 0x1f40)
                {
                    if (userSettings.SoundOutput.DataFormat != AIAudioFormatType.AIAUDIOTYPE_MULAW_8)
                    {
                        this.rad8KHz.Checked = true;
                    }
                    else
                    {
                        this.rad8KHzMuLaw.Checked = true;
                    }
                }
                else
                {
                    this.rad16KHz.Checked = true;
                }
            }
            this.chkFileHeader.Checked = userSettings.SoundOutput.FileHeader;
            this.chkSaveText.Checked = userSettings.SoundOutput.SaveText;
            this.btnApply.Enabled = false;
        }

        private void InitializeComponent()
        {
            this.btnApply = new Button();
            this.btnCancel = new Button();
            this.btnOK = new Button();
            this.grpWaveFormat = new GroupBox();
            this.chkFileHeader = new CheckBox();
            this.rad8KHzMuLaw = new RadioButton();
            this.rad8KHz = new RadioButton();
            this.rad11KHz = new RadioButton();
            this.rad16KHz = new RadioButton();
            this.rad22KHz = new RadioButton();
            this.btnInitialize = new Button();
            this.grp = new GroupBox();
            this.chkSaveText = new CheckBox();
            this.grpWaveFormat.SuspendLayout();
            this.grp.SuspendLayout();
            base.SuspendLayout();
            this.btnApply.Location = new Point(0x5d, 0x125);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new Size(0x4b, 0x17);
            this.btnApply.TabIndex = 5;
            this.btnApply.Text = "適用(A)";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Visible = false;
            this.btnCancel.DialogResult = DialogResult.Cancel;
            this.btnCancel.Location = new Point(0x183, 0x125);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new Size(0x4b, 0x17);
            this.btnCancel.TabIndex = 4;
            this.btnCancel.Text = "キャンセル";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnOK.Location = new Point(0x132, 0x125);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new Size(0x4b, 0x17);
            this.btnOK.TabIndex = 3;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.grpWaveFormat.Controls.Add(this.chkFileHeader);
            this.grpWaveFormat.Controls.Add(this.rad8KHzMuLaw);
            this.grpWaveFormat.Controls.Add(this.rad8KHz);
            this.grpWaveFormat.Controls.Add(this.rad11KHz);
            this.grpWaveFormat.Controls.Add(this.rad16KHz);
            this.grpWaveFormat.Controls.Add(this.rad22KHz);
            this.grpWaveFormat.Location = new Point(12, 12);
            this.grpWaveFormat.Name = "grpWaveFormat";
            this.grpWaveFormat.Size = new Size(450, 0x9b);
            this.grpWaveFormat.TabIndex = 1;
            this.grpWaveFormat.TabStop = false;
            this.grpWaveFormat.Text = "音声ファイル保存形式";
            this.chkFileHeader.AutoSize = true;
            this.chkFileHeader.Location = new Point(0x20, 0x73);
            this.chkFileHeader.Name = "chkFileHeader";
            this.chkFileHeader.Size = new Size(0x9d, 0x10);
            this.chkFileHeader.TabIndex = 6;
            this.chkFileHeader.Text = "音声ファイルにヘッダを付ける";
            this.chkFileHeader.UseVisualStyleBackColor = true;
            this.rad8KHzMuLaw.AutoSize = true;
            this.rad8KHzMuLaw.Location = new Point(0x107, 0x48);
            this.rad8KHzMuLaw.Name = "rad8KHzMuLaw";
            this.rad8KHzMuLaw.Size = new Size(0x58, 0x10);
            this.rad8KHzMuLaw.TabIndex = 5;
            this.rad8KHzMuLaw.TabStop = true;
            this.rad8KHzMuLaw.Text = "8KHz μ-law";
            this.rad8KHzMuLaw.UseVisualStyleBackColor = true;
            this.rad8KHz.AutoSize = true;
            this.rad8KHz.Location = new Point(0x98, 0x48);
            this.rad8KHz.Name = "rad8KHz";
            this.rad8KHz.Size = new Size(0x4d, 0x10);
            this.rad8KHz.TabIndex = 4;
            this.rad8KHz.TabStop = true;
            this.rad8KHz.Text = "8KHz PCM";
            this.rad8KHz.UseVisualStyleBackColor = true;
            this.rad11KHz.AutoSize = true;
            this.rad11KHz.Location = new Point(0x98, 0x24);
            this.rad11KHz.Name = "rad11KHz";
            this.rad11KHz.Size = new Size(0x53, 0x10);
            this.rad11KHz.TabIndex = 2;
            this.rad11KHz.TabStop = true;
            this.rad11KHz.Text = "11KHz PCM";
            this.rad11KHz.UseVisualStyleBackColor = true;
            this.rad16KHz.AutoSize = true;
            this.rad16KHz.Location = new Point(0x20, 0x48);
            this.rad16KHz.Name = "rad16KHz";
            this.rad16KHz.Size = new Size(0x53, 0x10);
            this.rad16KHz.TabIndex = 3;
            this.rad16KHz.TabStop = true;
            this.rad16KHz.Text = "16KHz PCM";
            this.rad16KHz.UseVisualStyleBackColor = true;
            this.rad22KHz.AutoSize = true;
            this.rad22KHz.Location = new Point(0x20, 0x24);
            this.rad22KHz.Name = "rad22KHz";
            this.rad22KHz.Size = new Size(0x53, 0x10);
            this.rad22KHz.TabIndex = 1;
            this.rad22KHz.TabStop = true;
            this.rad22KHz.Text = "22KHz PCM";
            this.rad22KHz.UseVisualStyleBackColor = true;
            this.btnInitialize.Location = new Point(12, 0x125);
            this.btnInitialize.Name = "btnInitialize";
            this.btnInitialize.Size = new Size(0x4b, 0x17);
            this.btnInitialize.TabIndex = 15;
            this.btnInitialize.Text = "初期設定";
            this.btnInitialize.UseVisualStyleBackColor = true;
            this.btnInitialize.Visible = false;
            this.grp.Controls.Add(this.chkSaveText);
            this.grp.Location = new Point(12, 0xad);
            this.grp.Name = "grp";
            this.grp.Size = new Size(450, 0x48);
            this.grp.TabIndex = 2;
            this.grp.TabStop = false;
            this.grp.Text = "その他";
            this.chkSaveText.AutoSize = true;
            this.chkSaveText.Location = new Point(0x20, 0x20);
            this.chkSaveText.Name = "chkSaveText";
            this.chkSaveText.Size = new Size(0xb3, 0x10);
            this.chkSaveText.TabIndex = 1;
            this.chkSaveText.Text = "テキストファイルを一緒に保存する";
            this.chkSaveText.UseVisualStyleBackColor = true;
            base.AutoScaleDimensions = new SizeF(6f, 12f);
            base.AutoScaleMode = AutoScaleMode.Font;
            base.CancelButton = this.btnCancel;
            base.ClientSize = new Size(0x1da, 0x148);
            base.Controls.Add(this.grp);
            base.Controls.Add(this.btnInitialize);
            base.Controls.Add(this.grpWaveFormat);
            base.Controls.Add(this.btnApply);
            base.Controls.Add(this.btnCancel);
            base.Controls.Add(this.btnOK);
            base.FormBorderStyle = FormBorderStyle.FixedDialog;
            base.MaximizeBox = false;
            base.MinimizeBox = false;
            base.Name = "SoundOutputSettingView";
            base.ShowIcon = false;
            base.ShowInTaskbar = false;
            base.StartPosition = FormStartPosition.CenterParent;
            this.Text = "音声ファイル保存設定";
            this.grpWaveFormat.ResumeLayout(false);
            this.grpWaveFormat.PerformLayout();
            this.grp.ResumeLayout(false);
            this.grp.PerformLayout();
            base.ResumeLayout(false);
        }

        public void UpdateSoundOutputSetting()
        {
            UserSettings.SoundOutputSettings args = new UserSettings.SoundOutputSettings();
            if (this.rad8KHzMuLaw.Checked || this.rad8KHz.Checked)
            {
                args.SamplePerSec = 0x1f40;
            }
            else if (this.rad11KHz.Checked)
            {
                args.SamplePerSec = 0x2b11;
            }
            else if (this.rad16KHz.Checked)
            {
                args.SamplePerSec = 0x3e80;
            }
            else
            {
                args.SamplePerSec = 0x5622;
            }
            args.DataFormat = this.rad8KHzMuLaw.Checked ? AIAudioFormatType.AIAUDIOTYPE_MULAW_8 : AIAudioFormatType.AIAUDIOTYPE_PCM_16;
            args.FileHeader = this.chkFileHeader.Checked;
            args.SaveText = this.chkSaveText.Checked;
            this._controller.UpdateSoundOutputSetting(args);
        }
    }
}

