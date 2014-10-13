namespace AITalkEditor
{
    using AITalkTuning;
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.IO;
    using System.Windows.Forms;

    public class LangDicSettingView : Form
    {
        private AppMode _appMode;
        private ILangDicSettingController _controller;
        private Button btnApply;
        private Button btnCancel;
        private Button btnInitialize;
        private Button btnLangPath;
        private Button btnOK;
        private Button btnPhraseDicPath;
        private Button btnSymbolDicPath;
        private Button btnWordDicPath;
        private CheckBox chkPhraseDic;
        private CheckBox chkSymbolDic;
        private CheckBox chkWordDic;
        private IContainer components;
        private GroupBox grpLang;
        private GroupBox grpPhraseDic;
        private GroupBox grpSymbolDic;
        private GroupBox grpWordDic;
        private Label lblLangPath;
        private Label lblPhraseDic;
        private Label lblSymbolDic;
        private Label lblWordDic;
        private TextBox txtLangPath;
        private TextBox txtPhraseDicPath;
        private TextBox txtSymbolDicPath;
        private TextBox txtWordDicPath;

        public LangDicSettingView(ILangDicSettingController c, AppSettings appSettings, UserSettings userSettings)
        {
            this.InitializeComponent();
            this._controller = c;
            this._appMode = appSettings.AppMode;
            this.btnLangPath.Click += new EventHandler(this.btnLangPath_Click);
            this.txtLangPath.TextChanged += new EventHandler(this.Controls_CheckedChanged);
            this.btnPhraseDicPath.Click += new EventHandler(this.btnPhraseDicPath_Click);
            this.btnWordDicPath.Click += new EventHandler(this.btnWordDicPath_Click);
            this.btnSymbolDicPath.Click += new EventHandler(this.btnSymbolDicPath_Click);
            this.txtPhraseDicPath.TextChanged += new EventHandler(this.Controls_CheckedChanged);
            this.txtWordDicPath.TextChanged += new EventHandler(this.Controls_CheckedChanged);
            this.txtSymbolDicPath.TextChanged += new EventHandler(this.Controls_CheckedChanged);
            this.chkPhraseDic.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.chkWordDic.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.chkSymbolDic.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.btnOK.Click += new EventHandler(this.btnOK_Click);
            this.btnCancel.Click += new EventHandler(this.btnCancel_Click);
            this.btnApply.Click += new EventHandler(this.btnApply_Click);
            this.Initialize(appSettings, userSettings);
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            this.UpdateLangDicSetting();
            this.btnApply.Enabled = false;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            base.DialogResult = DialogResult.Cancel;
        }

        private void btnLangPath_Click(object sender, EventArgs e)
        {
        Label_0000:
            using (FolderBrowserDialog dialog = new FolderBrowserDialog())
            {
                dialog.Description = "日本語辞書フォルダ(\"lang\")を選択してください。";
                dialog.SelectedPath = this.txtLangPath.Text;
                dialog.ShowNewFolderButton = false;
                if (DialogResult.OK == dialog.ShowDialog())
                {
                    if (!this.IsLangDic(dialog.SelectedPath))
                    {
                        MessageBox.Show(this, "指定されたフォルダには日本語辞書が含まれていません。", "エラー", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                        goto Label_0000;
                    }
                    this.txtLangPath.Text = dialog.SelectedPath;
                }
            }
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.UpdateLangDicSetting();
            base.DialogResult = DialogResult.OK;
        }

        private void btnPhraseDicPath_Click(object sender, EventArgs e)
        {
            this.SelectUserDicPath(this.txtPhraseDicPath, new UserDicType(typeof(PhraseDic), "フレーズ辞書", "pdic"));
        }

        private void btnSymbolDicPath_Click(object sender, EventArgs e)
        {
            this.SelectUserDicPath(this.txtSymbolDicPath, new UserDicType(typeof(SymbolDic), "記号ポーズ辞書", "sdic"));
        }

        private void btnWordDicPath_Click(object sender, EventArgs e)
        {
            this.SelectUserDicPath(this.txtWordDicPath, new UserDicType(typeof(WordDic), "単語辞書", "wdic"));
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
            this.txtLangPath.Text = userSettings.LangPath;
            this.chkPhraseDic.Checked = userSettings.UserDic.PhraseDicEnabled;
            this.txtPhraseDicPath.Text = userSettings.UserDic.PhraseDicPath;
            this.chkWordDic.Checked = userSettings.UserDic.WordDicEnabled;
            this.txtWordDicPath.Text = userSettings.UserDic.WordDicPath;
            this.chkSymbolDic.Checked = userSettings.UserDic.SymbolDicEnabled;
            this.txtSymbolDicPath.Text = userSettings.UserDic.SymbolDicPath;
            if (appSettings.Function.UseConstLangDic)
            {
                this.grpLang.Visible = false;
                int num = this.grpPhraseDic.Top - this.grpLang.Top;
                foreach (Control control in base.Controls)
                {
                    if (control.Top > this.grpLang.Top)
                    {
                        control.Top -= num;
                    }
                }
                base.Height -= num;
            }
            if (!AppModeUtil.CheckSupport(appSettings.AppMode, AppMode.PhraseDicSupport))
            {
                this.grpPhraseDic.Visible = false;
                int num2 = this.grpPhraseDic.Bottom - this.grpLang.Bottom;
                foreach (Control control2 in base.Controls)
                {
                    if (control2.Top > this.grpPhraseDic.Top)
                    {
                        control2.Top -= num2;
                    }
                }
                base.Height -= num2;
            }
            if (!AppModeUtil.CheckSupport(appSettings.AppMode, AppMode.WordDicSupport))
            {
                this.grpWordDic.Visible = false;
                int num3 = this.grpWordDic.Bottom - this.grpPhraseDic.Bottom;
                foreach (Control control3 in base.Controls)
                {
                    if (control3.Top > this.grpWordDic.Top)
                    {
                        control3.Top -= num3;
                    }
                }
                base.Height -= num3;
            }
            if (!AppModeUtil.CheckSupport(appSettings.AppMode, AppMode.SymbolDicSupport))
            {
                this.grpSymbolDic.Visible = false;
                int num4 = this.grpSymbolDic.Bottom - this.grpWordDic.Bottom;
                foreach (Control control4 in base.Controls)
                {
                    if (control4.Top > this.grpSymbolDic.Top)
                    {
                        control4.Top -= num4;
                    }
                }
                base.Height -= num4;
            }
            this.btnApply.Enabled = false;
        }

        private void InitializeComponent()
        {
            this.grpPhraseDic = new GroupBox();
            this.chkPhraseDic = new CheckBox();
            this.btnPhraseDicPath = new Button();
            this.txtPhraseDicPath = new TextBox();
            this.lblPhraseDic = new Label();
            this.grpWordDic = new GroupBox();
            this.chkWordDic = new CheckBox();
            this.btnWordDicPath = new Button();
            this.txtWordDicPath = new TextBox();
            this.lblWordDic = new Label();
            this.grpSymbolDic = new GroupBox();
            this.chkSymbolDic = new CheckBox();
            this.btnSymbolDicPath = new Button();
            this.txtSymbolDicPath = new TextBox();
            this.lblSymbolDic = new Label();
            this.btnOK = new Button();
            this.btnCancel = new Button();
            this.btnApply = new Button();
            this.grpLang = new GroupBox();
            this.lblLangPath = new Label();
            this.btnLangPath = new Button();
            this.txtLangPath = new TextBox();
            this.btnInitialize = new Button();
            this.grpPhraseDic.SuspendLayout();
            this.grpWordDic.SuspendLayout();
            this.grpSymbolDic.SuspendLayout();
            this.grpLang.SuspendLayout();
            base.SuspendLayout();
            this.grpPhraseDic.Controls.Add(this.chkPhraseDic);
            this.grpPhraseDic.Controls.Add(this.btnPhraseDicPath);
            this.grpPhraseDic.Controls.Add(this.txtPhraseDicPath);
            this.grpPhraseDic.Controls.Add(this.lblPhraseDic);
            this.grpPhraseDic.Location = new Point(12, 0x6a);
            this.grpPhraseDic.Name = "grpPhraseDic";
            this.grpPhraseDic.Size = new Size(530, 0x58);
            this.grpPhraseDic.TabIndex = 1;
            this.grpPhraseDic.TabStop = false;
            this.grpPhraseDic.Text = "フレーズ辞書";
            this.chkPhraseDic.AutoSize = true;
            this.chkPhraseDic.Location = new Point(0x76, 0x3a);
            this.chkPhraseDic.Name = "chkPhraseDic";
            this.chkPhraseDic.Size = new Size(0x89, 0x10);
            this.chkPhraseDic.TabIndex = 2;
            this.chkPhraseDic.Text = "フレーズ辞書を使用する";
            this.chkPhraseDic.UseVisualStyleBackColor = true;
            this.btnPhraseDicPath.Location = new Point(0x1b7, 0x33);
            this.btnPhraseDicPath.Name = "btnPhraseDicPath";
            this.btnPhraseDicPath.Size = new Size(0x4b, 0x17);
            this.btnPhraseDicPath.TabIndex = 3;
            this.btnPhraseDicPath.Text = "参照(P)...";
            this.btnPhraseDicPath.UseVisualStyleBackColor = true;
            this.txtPhraseDicPath.BackColor = SystemColors.Window;
            this.txtPhraseDicPath.Location = new Point(0x76, 0x19);
            this.txtPhraseDicPath.Name = "txtPhraseDicPath";
            this.txtPhraseDicPath.ReadOnly = true;
            this.txtPhraseDicPath.Size = new Size(0x18c, 0x13);
            this.txtPhraseDicPath.TabIndex = 1;
            this.txtPhraseDicPath.TabStop = false;
            this.lblPhraseDic.AutoSize = true;
            this.lblPhraseDic.Location = new Point(0x12, 0x1c);
            this.lblPhraseDic.Name = "lblPhraseDic";
            this.lblPhraseDic.Size = new Size(0x5c, 12);
            this.lblPhraseDic.TabIndex = 0;
            this.lblPhraseDic.Text = "ファイル名 (.pdic) :";
            this.grpWordDic.Controls.Add(this.chkWordDic);
            this.grpWordDic.Controls.Add(this.btnWordDicPath);
            this.grpWordDic.Controls.Add(this.txtWordDicPath);
            this.grpWordDic.Controls.Add(this.lblWordDic);
            this.grpWordDic.Location = new Point(12, 200);
            this.grpWordDic.Name = "grpWordDic";
            this.grpWordDic.Size = new Size(530, 0x58);
            this.grpWordDic.TabIndex = 2;
            this.grpWordDic.TabStop = false;
            this.grpWordDic.Text = "単語辞書";
            this.chkWordDic.AutoSize = true;
            this.chkWordDic.Location = new Point(0x76, 0x3a);
            this.chkWordDic.Name = "chkWordDic";
            this.chkWordDic.Size = new Size(0x7c, 0x10);
            this.chkWordDic.TabIndex = 2;
            this.chkWordDic.Text = "単語辞書を使用する";
            this.chkWordDic.UseVisualStyleBackColor = true;
            this.btnWordDicPath.Location = new Point(0x1b7, 50);
            this.btnWordDicPath.Name = "btnWordDicPath";
            this.btnWordDicPath.Size = new Size(0x4b, 0x17);
            this.btnWordDicPath.TabIndex = 3;
            this.btnWordDicPath.Text = "参照(W)...";
            this.btnWordDicPath.UseVisualStyleBackColor = true;
            this.txtWordDicPath.BackColor = SystemColors.Window;
            this.txtWordDicPath.Location = new Point(0x76, 0x19);
            this.txtWordDicPath.Name = "txtWordDicPath";
            this.txtWordDicPath.ReadOnly = true;
            this.txtWordDicPath.Size = new Size(0x18c, 0x13);
            this.txtWordDicPath.TabIndex = 1;
            this.txtWordDicPath.TabStop = false;
            this.lblWordDic.AutoSize = true;
            this.lblWordDic.Location = new Point(0x12, 0x1c);
            this.lblWordDic.Name = "lblWordDic";
            this.lblWordDic.Size = new Size(0x5e, 12);
            this.lblWordDic.TabIndex = 0;
            this.lblWordDic.Text = "ファイル名 (.wdic) :";
            this.grpSymbolDic.Controls.Add(this.chkSymbolDic);
            this.grpSymbolDic.Controls.Add(this.btnSymbolDicPath);
            this.grpSymbolDic.Controls.Add(this.txtSymbolDicPath);
            this.grpSymbolDic.Controls.Add(this.lblSymbolDic);
            this.grpSymbolDic.Location = new Point(12, 0x126);
            this.grpSymbolDic.Name = "grpSymbolDic";
            this.grpSymbolDic.Size = new Size(530, 0x58);
            this.grpSymbolDic.TabIndex = 3;
            this.grpSymbolDic.TabStop = false;
            this.grpSymbolDic.Text = "記号ポーズ辞書";
            this.chkSymbolDic.AutoSize = true;
            this.chkSymbolDic.Location = new Point(0x76, 0x3a);
            this.chkSymbolDic.Name = "chkSymbolDic";
            this.chkSymbolDic.Size = new Size(0x9a, 0x10);
            this.chkSymbolDic.TabIndex = 2;
            this.chkSymbolDic.Text = "記号ポーズ辞書を使用する";
            this.chkSymbolDic.UseVisualStyleBackColor = true;
            this.btnSymbolDicPath.Location = new Point(0x1b7, 0x36);
            this.btnSymbolDicPath.Name = "btnSymbolDicPath";
            this.btnSymbolDicPath.Size = new Size(0x4b, 0x17);
            this.btnSymbolDicPath.TabIndex = 3;
            this.btnSymbolDicPath.Text = "参照(S)...";
            this.btnSymbolDicPath.UseVisualStyleBackColor = true;
            this.txtSymbolDicPath.BackColor = SystemColors.Window;
            this.txtSymbolDicPath.Location = new Point(0x76, 0x19);
            this.txtSymbolDicPath.Name = "txtSymbolDicPath";
            this.txtSymbolDicPath.ReadOnly = true;
            this.txtSymbolDicPath.Size = new Size(0x18c, 0x13);
            this.txtSymbolDicPath.TabIndex = 1;
            this.txtSymbolDicPath.TabStop = false;
            this.lblSymbolDic.AutoSize = true;
            this.lblSymbolDic.Location = new Point(0x12, 0x1c);
            this.lblSymbolDic.Name = "lblSymbolDic";
            this.lblSymbolDic.Size = new Size(0x5c, 12);
            this.lblSymbolDic.TabIndex = 0;
            this.lblSymbolDic.Text = "ファイル名 (.sdic) :";
            this.btnOK.Location = new Point(0x182, 0x1a1);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new Size(0x4b, 0x17);
            this.btnOK.TabIndex = 4;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnCancel.DialogResult = DialogResult.Cancel;
            this.btnCancel.Location = new Point(0x1d3, 0x1a1);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new Size(0x4b, 0x17);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "キャンセル";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnApply.Location = new Point(0x5d, 0x1a1);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new Size(0x4b, 0x17);
            this.btnApply.TabIndex = 6;
            this.btnApply.Text = "適用(A)";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Visible = false;
            this.grpLang.Controls.Add(this.lblLangPath);
            this.grpLang.Controls.Add(this.btnLangPath);
            this.grpLang.Controls.Add(this.txtLangPath);
            this.grpLang.Location = new Point(12, 12);
            this.grpLang.Name = "grpLang";
            this.grpLang.Size = new Size(530, 0x58);
            this.grpLang.TabIndex = 0;
            this.grpLang.TabStop = false;
            this.grpLang.Text = "標準辞書";
            this.lblLangPath.AutoSize = true;
            this.lblLangPath.Location = new Point(0x12, 0x1c);
            this.lblLangPath.Name = "lblLangPath";
            this.lblLangPath.Size = new Size(0x5b, 12);
            this.lblLangPath.TabIndex = 0;
            this.lblLangPath.Text = "フォルダ名 (lang) :";
            this.btnLangPath.Location = new Point(0x1b7, 50);
            this.btnLangPath.Name = "btnLangPath";
            this.btnLangPath.Size = new Size(0x4b, 0x17);
            this.btnLangPath.TabIndex = 2;
            this.btnLangPath.Text = "参照(L)...";
            this.btnLangPath.UseVisualStyleBackColor = true;
            this.txtLangPath.BackColor = SystemColors.Window;
            this.txtLangPath.Location = new Point(0x76, 0x19);
            this.txtLangPath.Name = "txtLangPath";
            this.txtLangPath.ReadOnly = true;
            this.txtLangPath.Size = new Size(0x18c, 0x13);
            this.txtLangPath.TabIndex = 1;
            this.txtLangPath.TabStop = false;
            this.btnInitialize.Location = new Point(12, 0x1a1);
            this.btnInitialize.Name = "btnInitialize";
            this.btnInitialize.Size = new Size(0x4b, 0x17);
            this.btnInitialize.TabIndex = 7;
            this.btnInitialize.Text = "初期設定";
            this.btnInitialize.UseVisualStyleBackColor = true;
            this.btnInitialize.Visible = false;
            base.AutoScaleDimensions = new SizeF(6f, 12f);
            base.AutoScaleMode = AutoScaleMode.Font;
            base.CancelButton = this.btnCancel;
            base.ClientSize = new Size(0x22a, 0x1c4);
            base.Controls.Add(this.btnInitialize);
            base.Controls.Add(this.grpLang);
            base.Controls.Add(this.btnApply);
            base.Controls.Add(this.btnCancel);
            base.Controls.Add(this.btnOK);
            base.Controls.Add(this.grpSymbolDic);
            base.Controls.Add(this.grpWordDic);
            base.Controls.Add(this.grpPhraseDic);
            base.FormBorderStyle = FormBorderStyle.FixedDialog;
            base.MaximizeBox = false;
            base.MinimizeBox = false;
            base.Name = "LangDicSettingView";
            base.ShowIcon = false;
            base.ShowInTaskbar = false;
            base.StartPosition = FormStartPosition.CenterParent;
            this.Text = "日本語辞書設定";
            this.grpPhraseDic.ResumeLayout(false);
            this.grpPhraseDic.PerformLayout();
            this.grpWordDic.ResumeLayout(false);
            this.grpWordDic.PerformLayout();
            this.grpSymbolDic.ResumeLayout(false);
            this.grpSymbolDic.PerformLayout();
            this.grpLang.ResumeLayout(false);
            this.grpLang.PerformLayout();
            base.ResumeLayout(false);
        }

        private bool IsLangDic(string langPath)
        {
            string[] langDicRequiredFilesStandard;
            if (!AppModeUtil.CheckSupport(this._appMode, AppMode.MicroLang))
            {
                langDicRequiredFilesStandard = Common.LangDicRequiredFilesStandard;
            }
            else
            {
                langDicRequiredFilesStandard = Common.LangDicRequiredFilesMicro;
            }
            foreach (string str in langDicRequiredFilesStandard)
            {
                if (!File.Exists(Path.Combine(langPath, str)))
                {
                    return false;
                }
            }
            return true;
        }

        private void ofd_FileOk(object sender, CancelEventArgs e)
        {
            OpenFileDialog dialog = (OpenFileDialog) sender;
            UserDicType tag = (UserDicType) dialog.Tag;
            if (!File.Exists(dialog.FileName))
            {
                if (DialogResult.Yes == MessageBox.Show("ファイル " + dialog.FileName + " は存在しません。" + Environment.NewLine + "新規作成しますか？", tag.Name + "の作成", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button2))
                {
                    try
                    {
                        if (tag.DataType == typeof(PhraseDic))
                        {
                            new PhraseDic().Write(dialog.FileName);
                        }
                        else if (tag.DataType == typeof(WordDic))
                        {
                            new WordDic().Write(dialog.FileName);
                        }
                        else if (tag.DataType == typeof(SymbolDic))
                        {
                            new SymbolDic().Write(dialog.FileName);
                        }
                    }
                    catch (Exception exception)
                    {
                        MessageBox.Show(this, tag.Name + "の作成に失敗しました。" + Environment.NewLine + exception.Message, "エラー", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                        e.Cancel = true;
                    }
                }
                else
                {
                    e.Cancel = true;
                }
            }
        }

        private void SelectUserDicPath(TextBox txtDicPath, UserDicType type)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.FileOk += new CancelEventHandler(this.ofd_FileOk);
            dialog.CheckFileExists = false;
            dialog.CheckPathExists = true;
            dialog.DefaultExt = type.Extension;
            dialog.FileName = Path.GetFileName(txtDicPath.Text);
            dialog.Filter = type.Name + " (*." + type.Extension + ")|*." + type.Extension + "|すべてのファイル (*.*)|*.*";
            dialog.InitialDirectory = Path.GetDirectoryName(txtDicPath.Text);
            dialog.RestoreDirectory = true;
            dialog.Title = type.Name + "の選択";
            dialog.Tag = type;
            if (DialogResult.OK == dialog.ShowDialog())
            {
                txtDicPath.Text = dialog.FileName;
            }
            dialog.Dispose();
        }

        private void UpdateLangDicSetting()
        {
            this._controller.UpdateLangDicSetting(new LangDicSettingArgs(this.txtLangPath.Text, this.chkWordDic.Checked, this.chkPhraseDic.Checked, this.chkSymbolDic.Checked, this.txtWordDicPath.Text, this.txtPhraseDicPath.Text, this.txtSymbolDicPath.Text));
        }
    }
}

