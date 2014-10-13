namespace AITalkEditor
{
    using AITalkEditor.Properties;
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Drawing;
    using System.IO;
    using System.Windows.Forms;

    public class VoiceDicSettingView : Form
    {
        private IVoiceDicSettingController _controller;
        private UserSettings _userSettings;
        private Button btnApply;
        private Button btnCancel;
        private Button btnDbsPath;
        private Button btnInitialize;
        private Button btnOK;
        private IContainer components;
        private GroupBox grpDbs;
        private Label lblDbsPath;
        private Label lblSamplesPerSec;
        private Label lblVoiceList;
        private RadioButton rad16KHz;
        private RadioButton rad22KHz;
        private TextBox txtDbsPath;
        private VoiceListView voiceListView;

        public VoiceDicSettingView(IVoiceDicSettingController c, UserSettings userSettings)
        {
            this.InitializeComponent();
            this._controller = c;
            this._userSettings = userSettings;
            base.Shown += new EventHandler(this.VoiceDicSettingView_Shown);
            this.btnDbsPath.Click += new EventHandler(this.btnDbsPath_Click);
            this.txtDbsPath.TextChanged += new EventHandler(this.txtDbsPath_TextChanged);
            this.rad22KHz.CheckedChanged += new EventHandler(this.rad22KHz_CheckedChanged);
            this.rad16KHz.CheckedChanged += new EventHandler(this.rad16KHz_CheckedChanged);
            this.btnOK.Click += new EventHandler(this.btnOK_Click);
            this.btnCancel.Click += new EventHandler(this.btnCancel_Click);
            this.btnApply.Click += new EventHandler(this.btnApply_Click);
            this.voiceListView.SelectionChanged += new EventHandler(this.voiceListView_SelectionChanged);
            this.voiceListView.CellLoadChanged += new DataGridViewCellEventHandler(this.voiceListView_CellLoadChanged);
            this.voiceListView.GenerateColumns();
            this.Initialize();
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            this.UpdateVoiceDicSetting();
            this.btnApply.Enabled = false;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            base.DialogResult = DialogResult.Cancel;
        }

        private void btnDbsPath_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog {
                Description = "音声辞書フォルダ(\"voice\")を選択してください。",
                SelectedPath = this.txtDbsPath.Text,
                ShowNewFolderButton = false
            };
            if (DialogResult.OK == dialog.ShowDialog())
            {
                this.txtDbsPath.Text = dialog.SelectedPath;
            }
            dialog.Dispose();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.UpdateVoiceDicSetting();
            base.DialogResult = DialogResult.OK;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing && (this.components != null))
            {
                this.components.Dispose();
            }
            base.Dispose(disposing);
        }

        public void Initialize()
        {
            this.txtDbsPath.Text = this._userSettings.DbsPath;
            if (this._userSettings.VoiceSamplePerSec != 0x3e80)
            {
                this.rad22KHz.Checked = true;
            }
            else
            {
                this.rad16KHz.Checked = true;
            }
        }

        private void InitializeComponent()
        {
            this.btnApply = new Button();
            this.btnCancel = new Button();
            this.btnOK = new Button();
            this.grpDbs = new GroupBox();
            this.lblVoiceList = new Label();
            this.voiceListView = new VoiceListView();
            this.lblSamplesPerSec = new Label();
            this.lblDbsPath = new Label();
            this.rad16KHz = new RadioButton();
            this.rad22KHz = new RadioButton();
            this.btnDbsPath = new Button();
            this.txtDbsPath = new TextBox();
            this.btnInitialize = new Button();
            this.grpDbs.SuspendLayout();
            ((ISupportInitialize) this.voiceListView).BeginInit();
            base.SuspendLayout();
            this.btnApply.Location = new Point(0x5d, 0x1a1);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new Size(0x4b, 0x17);
            this.btnApply.TabIndex = 6;
            this.btnApply.Text = "適用(A)";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Visible = false;
            this.btnCancel.DialogResult = DialogResult.Cancel;
            this.btnCancel.Location = new Point(0x1d3, 0x1a1);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new Size(0x4b, 0x17);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "キャンセル";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnOK.Location = new Point(0x182, 0x1a1);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new Size(0x4b, 0x17);
            this.btnOK.TabIndex = 4;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.grpDbs.Controls.Add(this.lblVoiceList);
            this.grpDbs.Controls.Add(this.voiceListView);
            this.grpDbs.Controls.Add(this.lblSamplesPerSec);
            this.grpDbs.Controls.Add(this.lblDbsPath);
            this.grpDbs.Controls.Add(this.rad16KHz);
            this.grpDbs.Controls.Add(this.rad22KHz);
            this.grpDbs.Controls.Add(this.btnDbsPath);
            this.grpDbs.Controls.Add(this.txtDbsPath);
            this.grpDbs.Location = new Point(12, 12);
            this.grpDbs.Name = "grpDbs";
            this.grpDbs.Size = new Size(530, 0x16e);
            this.grpDbs.TabIndex = 1;
            this.grpDbs.TabStop = false;
            this.grpDbs.Text = "音声辞書";
            this.lblVoiceList.AutoSize = true;
            this.lblVoiceList.Location = new Point(0x12, 0x7d);
            this.lblVoiceList.Name = "lblVoiceList";
            this.lblVoiceList.Size = new Size(0x4e, 12);
            this.lblVoiceList.TabIndex = 6;
            this.lblVoiceList.Text = "使用する話者 :";
            this.voiceListView.AllowUserToAddRows = false;
            this.voiceListView.AllowUserToDeleteRows = false;
            this.voiceListView.AllowUserToResizeColumns = false;
            this.voiceListView.AllowUserToResizeRows = false;
            this.voiceListView.CellBorderStyle = DataGridViewCellBorderStyle.SingleHorizontal;
            this.voiceListView.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.voiceListView.Location = new Point(0x72, 0x7d);
            this.voiceListView.MultiSelect = false;
            this.voiceListView.Name = "voiceListView";
            this.voiceListView.RowHeadersVisible = false;
            this.voiceListView.RowTemplate.Height = 0x30;
            this.voiceListView.ScrollBars = ScrollBars.Vertical;
            this.voiceListView.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            this.voiceListView.Size = new Size(0x18f, 220);
            this.voiceListView.TabIndex = 5;
            this.lblSamplesPerSec.AutoSize = true;
            this.lblSamplesPerSec.Location = new Point(20, 0x51);
            this.lblSamplesPerSec.Name = "lblSamplesPerSec";
            this.lblSamplesPerSec.Size = new Size(0x4c, 12);
            this.lblSamplesPerSec.TabIndex = 0;
            this.lblSamplesPerSec.Text = "サンプルレート :";
            this.lblDbsPath.AutoSize = true;
            this.lblDbsPath.Location = new Point(0x12, 0x23);
            this.lblDbsPath.Name = "lblDbsPath";
            this.lblDbsPath.Size = new Size(0x61, 12);
            this.lblDbsPath.TabIndex = 0;
            this.lblDbsPath.Text = "フォルダ名 (voice) :";
            this.rad16KHz.AutoSize = true;
            this.rad16KHz.Location = new Point(0xcd, 0x4f);
            this.rad16KHz.Name = "rad16KHz";
            this.rad16KHz.Size = new Size(0x37, 0x10);
            this.rad16KHz.TabIndex = 4;
            this.rad16KHz.TabStop = true;
            this.rad16KHz.Text = "16KHz";
            this.rad16KHz.UseVisualStyleBackColor = true;
            this.rad22KHz.AutoSize = true;
            this.rad22KHz.Location = new Point(0x72, 0x4f);
            this.rad22KHz.Name = "rad22KHz";
            this.rad22KHz.Size = new Size(0x37, 0x10);
            this.rad22KHz.TabIndex = 3;
            this.rad22KHz.TabStop = true;
            this.rad22KHz.Text = "22KHz";
            this.rad22KHz.UseVisualStyleBackColor = true;
            this.btnDbsPath.Location = new Point(0x1b6, 0x39);
            this.btnDbsPath.Name = "btnDbsPath";
            this.btnDbsPath.Size = new Size(0x4b, 0x17);
            this.btnDbsPath.TabIndex = 2;
            this.btnDbsPath.Text = "参照(D)...";
            this.btnDbsPath.UseVisualStyleBackColor = true;
            this.txtDbsPath.BackColor = SystemColors.Window;
            this.txtDbsPath.Location = new Point(0x79, 0x20);
            this.txtDbsPath.Name = "txtDbsPath";
            this.txtDbsPath.ReadOnly = true;
            this.txtDbsPath.Size = new Size(0x188, 0x13);
            this.txtDbsPath.TabIndex = 1;
            this.txtDbsPath.TabStop = false;
            this.btnInitialize.Location = new Point(12, 0x1a1);
            this.btnInitialize.Name = "btnInitialize";
            this.btnInitialize.Size = new Size(0x4b, 0x17);
            this.btnInitialize.TabIndex = 15;
            this.btnInitialize.Text = "初期設定";
            this.btnInitialize.UseVisualStyleBackColor = true;
            this.btnInitialize.Visible = false;
            base.AutoScaleDimensions = new SizeF(6f, 12f);
            base.AutoScaleMode = AutoScaleMode.Font;
            base.CancelButton = this.btnCancel;
            base.ClientSize = new Size(0x22a, 0x1c4);
            base.Controls.Add(this.btnInitialize);
            base.Controls.Add(this.grpDbs);
            base.Controls.Add(this.btnApply);
            base.Controls.Add(this.btnCancel);
            base.Controls.Add(this.btnOK);
            base.FormBorderStyle = FormBorderStyle.FixedDialog;
            base.MaximizeBox = false;
            base.MinimizeBox = false;
            base.Name = "VoiceDicSettingView";
            base.ShowIcon = false;
            base.ShowInTaskbar = false;
            base.StartPosition = FormStartPosition.CenterParent;
            this.Text = "音声辞書設定";
            this.grpDbs.ResumeLayout(false);
            this.grpDbs.PerformLayout();
            ((ISupportInitialize) this.voiceListView).EndInit();
            base.ResumeLayout(false);
        }

        private void rad16KHz_CheckedChanged(object sender, EventArgs e)
        {
            this.RefreshVoiceListView();
            this.btnApply.Enabled = true;
        }

        private void rad22KHz_CheckedChanged(object sender, EventArgs e)
        {
            this.RefreshVoiceListView();
            this.btnApply.Enabled = true;
        }

        private void RefreshVoiceListView()
        {
            this.voiceListView.Rows.Clear();
            string text = this.txtDbsPath.Text;
            List<VoiceInfo> list = this._controller.EnumVoiceNames(text, this.rad16KHz.Checked ? 0x3e80 : 0x5622);
            int num = 0;
            foreach (VoiceInfo info in list)
            {
                Bitmap noimage;
                try
                {
                    noimage = new Bitmap(Path.Combine(Path.Combine(text, info.DirectoryName), Common.VoiceImageFileName));
                }
                catch
                {
                    noimage = Resources.noimage;
                }
                bool flag = this._controller.CheckIgnoredVoiceName(info.DirectoryName, this._userSettings.IgnoredVoiceNames);
                this.voiceListView.AddRow(info.DirectoryName, noimage, info.DisplayName, !flag);
                if (info.DirectoryName == this._userSettings.SelectedVoiceName)
                {
                    this.voiceListView.Rows[num].Selected = true;
                }
                num++;
            }
        }

        private void txtDbsPath_TextChanged(object sender, EventArgs e)
        {
            this.RefreshVoiceListView();
            this.btnApply.Enabled = true;
        }

        public void UpdateVoiceDicSetting()
        {
            List<string> ignoredVoiceNames = new List<string>();
            string selectecVoiceName = "";
            foreach (DataGridViewRow row in (IEnumerable) this.voiceListView.Rows)
            {
                string item = (string) row.Cells[3].Value;
                if (!((bool) row.Cells[2].Value))
                {
                    ignoredVoiceNames.Add(item);
                }
                if (row.Selected)
                {
                    selectecVoiceName = item;
                }
            }
            this._controller.UpdateVoiceDicSetting(new VoiceDicSettingArgs(this.txtDbsPath.Text, !this.rad16KHz.Checked ? 0x5622 : 0x3e80, ignoredVoiceNames, selectecVoiceName));
        }

        private void VoiceDicSettingView_Shown(object sender, EventArgs e)
        {
            this.RefreshVoiceListView();
            this.btnApply.Enabled = false;
        }

        private void voiceListView_CellLoadChanged(object sender, DataGridViewCellEventArgs e)
        {
            this.btnApply.Enabled = true;
        }

        private void voiceListView_SelectionChanged(object sender, EventArgs e)
        {
            if (this.voiceListView.SelectedRows.Count == 1)
            {
                this.btnApply.Enabled = true;
            }
        }
    }
}

