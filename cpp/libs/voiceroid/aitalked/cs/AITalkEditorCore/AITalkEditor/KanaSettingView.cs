namespace AITalkEditor
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;

    public class KanaSettingView : Form
    {
        private IKanaSettingController _controller;
        private Button btnApply;
        private Button btnCancel;
        private Button btnInitialize;
        private Button btnOK;
        private ImageComboBox cmbJeitaFemale;
        private ImageComboBox cmbJeitaMale;
        private IContainer components;
        private GroupBox grpKanaFormat;
        private Label label3;
        private Label lblJeitaFemale;
        private Label lblJeitaMale;
        private RadioButton radAIKana;
        private RadioButton radJeita;

        public KanaSettingView(IKanaSettingController c, UserSettings userSettings)
        {
            this.InitializeComponent();
            this._controller = c;
            this.radAIKana.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radJeita.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.btnOK.Click += new EventHandler(this.btnOK_Click);
            this.btnCancel.Click += new EventHandler(this.btnCancel_Click);
            this.btnApply.Click += new EventHandler(this.btnApply_Click);
            this.Initialize(userSettings);
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            this._controller.UpdateKanaSetting();
            this.btnApply.Enabled = false;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            base.DialogResult = DialogResult.Cancel;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this._controller.UpdateKanaSetting();
            base.DialogResult = DialogResult.OK;
        }

        private void Controls_CheckedChanged(object sender, EventArgs e)
        {
            if (this.radAIKana.Checked)
            {
                this.cmbJeitaFemale.Enabled = false;
                this.cmbJeitaMale.Enabled = false;
            }
            else
            {
                this.cmbJeitaFemale.Enabled = true;
                this.cmbJeitaMale.Enabled = true;
            }
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

        public void Initialize(UserSettings userSettings)
        {
            if (userSettings.KanaMode != UserSettings.KanaFormat.JEITA)
            {
                this.radAIKana.Checked = true;
            }
            else
            {
                this.radJeita.Checked = true;
            }
            this.btnApply.Enabled = false;
        }

        private void InitializeComponent()
        {
            this.btnApply = new Button();
            this.btnCancel = new Button();
            this.btnOK = new Button();
            this.grpKanaFormat = new GroupBox();
            this.label3 = new Label();
            this.lblJeitaMale = new Label();
            this.lblJeitaFemale = new Label();
            this.cmbJeitaMale = new ImageComboBox();
            this.cmbJeitaFemale = new ImageComboBox();
            this.radJeita = new RadioButton();
            this.radAIKana = new RadioButton();
            this.btnInitialize = new Button();
            this.grpKanaFormat.SuspendLayout();
            base.SuspendLayout();
            this.btnApply.Location = new Point(0x5d, 0x151);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new Size(0x4b, 0x17);
            this.btnApply.TabIndex = 3;
            this.btnApply.Text = "適用(A)";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnCancel.DialogResult = DialogResult.Cancel;
            this.btnCancel.Location = new Point(0x183, 0x151);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new Size(0x4b, 0x17);
            this.btnCancel.TabIndex = 2;
            this.btnCancel.Text = "キャンセル";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnOK.Location = new Point(0x132, 0x151);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new Size(0x4b, 0x17);
            this.btnOK.TabIndex = 1;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.grpKanaFormat.Controls.Add(this.label3);
            this.grpKanaFormat.Controls.Add(this.lblJeitaMale);
            this.grpKanaFormat.Controls.Add(this.lblJeitaFemale);
            this.grpKanaFormat.Controls.Add(this.cmbJeitaMale);
            this.grpKanaFormat.Controls.Add(this.cmbJeitaFemale);
            this.grpKanaFormat.Controls.Add(this.radJeita);
            this.grpKanaFormat.Controls.Add(this.radAIKana);
            this.grpKanaFormat.Location = new Point(12, 12);
            this.grpKanaFormat.Name = "grpKanaFormat";
            this.grpKanaFormat.Size = new Size(450, 0xd9);
            this.grpKanaFormat.TabIndex = 0;
            this.grpKanaFormat.TabStop = false;
            this.grpKanaFormat.Text = "中間言語形式";
            this.label3.AutoSize = true;
            this.label3.Location = new Point(0x12, 0x23);
            this.label3.Name = "label3";
            this.label3.Size = new Size(0x53, 12);
            this.label3.TabIndex = 0;
            this.label3.Text = "中間言語形式 :";
            this.lblJeitaMale.AutoSize = true;
            this.lblJeitaMale.Location = new Point(0x12, 0x85);
            this.lblJeitaMale.Name = "lblJeitaMale";
            this.lblJeitaMale.Size = new Size(80, 12);
            this.lblJeitaMale.TabIndex = 5;
            this.lblJeitaMale.Text = "男性話者 (M) :";
            this.lblJeitaFemale.AutoSize = true;
            this.lblJeitaFemale.Location = new Point(0x12, 0x4d);
            this.lblJeitaFemale.Name = "lblJeitaFemale";
            this.lblJeitaFemale.Size = new Size(0x4e, 12);
            this.lblJeitaFemale.TabIndex = 3;
            this.lblJeitaFemale.Text = "女性話者 (F) :";
            this.cmbJeitaMale.DrawMode = DrawMode.OwnerDrawFixed;
            this.cmbJeitaMale.DropDownStyle = ComboBoxStyle.DropDownList;
            this.cmbJeitaMale.FormattingEnabled = true;
            this.cmbJeitaMale.ItemHeight = 0x2c;
            this.cmbJeitaMale.Location = new Point(0x6b, 0x80);
            this.cmbJeitaMale.Name = "cmbJeitaMale";
            this.cmbJeitaMale.Size = new Size(160, 50);
            this.cmbJeitaMale.TabIndex = 6;
            this.cmbJeitaMale.TextAlign = VerticalAlignment.Top;
            this.cmbJeitaMale.TextX = 0;
            this.cmbJeitaFemale.DrawMode = DrawMode.OwnerDrawFixed;
            this.cmbJeitaFemale.DropDownStyle = ComboBoxStyle.DropDownList;
            this.cmbJeitaFemale.FormattingEnabled = true;
            this.cmbJeitaFemale.ItemHeight = 0x2c;
            this.cmbJeitaFemale.Location = new Point(0x6b, 0x48);
            this.cmbJeitaFemale.Name = "cmbJeitaFemale";
            this.cmbJeitaFemale.Size = new Size(160, 50);
            this.cmbJeitaFemale.TabIndex = 4;
            this.cmbJeitaFemale.TextAlign = VerticalAlignment.Top;
            this.cmbJeitaFemale.TextX = 0;
            this.radJeita.AutoSize = true;
            this.radJeita.Location = new Point(0xc4, 0x1f);
            this.radJeita.Name = "radJeita";
            this.radJeita.Size = new Size(0x67, 0x10);
            this.radJeita.TabIndex = 2;
            this.radJeita.TabStop = true;
            this.radJeita.Text = "JEITA TT-6004";
            this.radJeita.UseVisualStyleBackColor = true;
            this.radAIKana.AutoSize = true;
            this.radAIKana.Location = new Point(0x6b, 0x1f);
            this.radAIKana.Name = "radAIKana";
            this.radAIKana.Size = new Size(0x41, 0x10);
            this.radAIKana.TabIndex = 1;
            this.radAIKana.TabStop = true;
            this.radAIKana.Text = "AIKANA";
            this.radAIKana.UseVisualStyleBackColor = true;
            this.btnInitialize.Location = new Point(12, 0x151);
            this.btnInitialize.Name = "btnInitialize";
            this.btnInitialize.Size = new Size(0x4b, 0x17);
            this.btnInitialize.TabIndex = 4;
            this.btnInitialize.Text = "初期設定";
            this.btnInitialize.UseVisualStyleBackColor = true;
            this.btnInitialize.Visible = false;
            base.AutoScaleDimensions = new SizeF(6f, 12f);
            base.AutoScaleMode = AutoScaleMode.Font;
            base.CancelButton = this.btnCancel;
            base.ClientSize = new Size(0x1da, 0x174);
            base.Controls.Add(this.btnInitialize);
            base.Controls.Add(this.grpKanaFormat);
            base.Controls.Add(this.btnApply);
            base.Controls.Add(this.btnCancel);
            base.Controls.Add(this.btnOK);
            base.FormBorderStyle = FormBorderStyle.FixedDialog;
            base.MaximizeBox = false;
            base.MinimizeBox = false;
            base.Name = "KanaSettingView";
            base.ShowIcon = false;
            base.ShowInTaskbar = false;
            base.StartPosition = FormStartPosition.CenterParent;
            this.Text = "中間言語設定";
            this.grpKanaFormat.ResumeLayout(false);
            this.grpKanaFormat.PerformLayout();
            base.ResumeLayout(false);
        }
    }
}

