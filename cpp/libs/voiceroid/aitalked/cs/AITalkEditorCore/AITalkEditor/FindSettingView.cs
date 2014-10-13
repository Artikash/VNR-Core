namespace AITalkEditor
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;

    public class FindSettingView : Form
    {
        private IFindSettingController _controller;
        private Button btnApply;
        private Button btnCancel;
        private Button btnInitialize;
        private Button btnOK;
        private IContainer components;
        private GroupBox grpLogicalCondition;
        private GroupBox grpMatchingCondition;
        private GroupBox grpPaging;
        private GroupBox grpTargetField;
        private Label lblPageSize;
        private NumericUpDown numPageSize;
        private RadioButton radAnd;
        private RadioButton radBackward;
        private RadioButton radForward;
        private RadioButton radOr;
        private RadioButton radPartial;
        private RadioButton radText;
        private RadioButton radYomi;

        public FindSettingView(IFindSettingController c, UserSettings userSettings)
        {
            this.InitializeComponent();
            this._controller = c;
            this.radText.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radYomi.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radAnd.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radOr.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radForward.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radPartial.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.radBackward.CheckedChanged += new EventHandler(this.Controls_CheckedChanged);
            this.numPageSize.ValueChanged += new EventHandler(this.Controls_CheckedChanged);
            this.btnOK.Click += new EventHandler(this.btnOK_Click);
            this.btnCancel.Click += new EventHandler(this.btnCancel_Click);
            this.btnApply.Click += new EventHandler(this.btnApply_Click);
            this.Initialize(userSettings);
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            this.UpdateFindSetting();
            this.btnApply.Enabled = false;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            base.DialogResult = DialogResult.Cancel;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.UpdateFindSetting();
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

        public void Initialize(UserSettings userSettings)
        {
            if (userSettings.Find.Target != UserSettings.FindSettings.TargetField.Yomi)
            {
                this.radText.Checked = true;
            }
            else
            {
                this.radYomi.Checked = true;
            }
            if (userSettings.Find.Logic != UserSettings.FindSettings.LogicalCondition.Or)
            {
                this.radAnd.Checked = true;
            }
            else
            {
                this.radOr.Checked = true;
            }
            if (userSettings.Find.Match == UserSettings.FindSettings.MatchingCondition.Forward)
            {
                this.radForward.Checked = true;
            }
            else if (userSettings.Find.Match == UserSettings.FindSettings.MatchingCondition.Backward)
            {
                this.radBackward.Checked = true;
            }
            else
            {
                this.radPartial.Checked = true;
            }
            this.numPageSize.Value = userSettings.Find.PageSize;
            this.btnApply.Enabled = false;
        }

        private void InitializeComponent()
        {
            this.grpTargetField = new GroupBox();
            this.radYomi = new RadioButton();
            this.radText = new RadioButton();
            this.grpLogicalCondition = new GroupBox();
            this.radOr = new RadioButton();
            this.radAnd = new RadioButton();
            this.grpMatchingCondition = new GroupBox();
            this.radBackward = new RadioButton();
            this.radPartial = new RadioButton();
            this.radForward = new RadioButton();
            this.grpPaging = new GroupBox();
            this.lblPageSize = new Label();
            this.numPageSize = new NumericUpDown();
            this.btnApply = new Button();
            this.btnCancel = new Button();
            this.btnOK = new Button();
            this.btnInitialize = new Button();
            this.grpTargetField.SuspendLayout();
            this.grpLogicalCondition.SuspendLayout();
            this.grpMatchingCondition.SuspendLayout();
            this.grpPaging.SuspendLayout();
            this.numPageSize.BeginInit();
            base.SuspendLayout();
            this.grpTargetField.Controls.Add(this.radYomi);
            this.grpTargetField.Controls.Add(this.radText);
            this.grpTargetField.Location = new Point(12, 12);
            this.grpTargetField.Name = "grpTargetField";
            this.grpTargetField.Size = new Size(440, 0x40);
            this.grpTargetField.TabIndex = 0;
            this.grpTargetField.TabStop = false;
            this.grpTargetField.Text = "検索対象となる項目";
            this.radYomi.AutoSize = true;
            this.radYomi.Location = new Point(0xb3, 0x1c);
            this.radYomi.Name = "radYomi";
            this.radYomi.Size = new Size(0x2e, 0x10);
            this.radYomi.TabIndex = 1;
            this.radYomi.TabStop = true;
            this.radYomi.Text = "読み";
            this.radYomi.UseVisualStyleBackColor = true;
            this.radText.AutoSize = true;
            this.radText.Location = new Point(0x20, 0x1c);
            this.radText.Name = "radText";
            this.radText.Size = new Size(0x38, 0x10);
            this.radText.TabIndex = 0;
            this.radText.TabStop = true;
            this.radText.Text = "見出し";
            this.radText.UseVisualStyleBackColor = true;
            this.grpLogicalCondition.Controls.Add(this.radOr);
            this.grpLogicalCondition.Controls.Add(this.radAnd);
            this.grpLogicalCondition.Location = new Point(12, 0x52);
            this.grpLogicalCondition.Name = "grpLogicalCondition";
            this.grpLogicalCondition.Size = new Size(440, 0x40);
            this.grpLogicalCondition.TabIndex = 1;
            this.grpLogicalCondition.TabStop = false;
            this.grpLogicalCondition.Text = "複数キーワード指定時の検索条件";
            this.radOr.AutoSize = true;
            this.radOr.Location = new Point(0xb3, 0x1c);
            this.radOr.Name = "radOr";
            this.radOr.Size = new Size(0x53, 0x10);
            this.radOr.TabIndex = 1;
            this.radOr.TabStop = true;
            this.radOr.Text = "OR （または）";
            this.radOr.UseVisualStyleBackColor = true;
            this.radAnd.AutoSize = true;
            this.radAnd.Location = new Point(0x20, 0x1c);
            this.radAnd.Name = "radAnd";
            this.radAnd.Size = new Size(0x52, 0x10);
            this.radAnd.TabIndex = 0;
            this.radAnd.TabStop = true;
            this.radAnd.Text = "AND （かつ）";
            this.radAnd.UseVisualStyleBackColor = true;
            this.grpMatchingCondition.Controls.Add(this.radBackward);
            this.grpMatchingCondition.Controls.Add(this.radPartial);
            this.grpMatchingCondition.Controls.Add(this.radForward);
            this.grpMatchingCondition.Location = new Point(12, 0x98);
            this.grpMatchingCondition.Name = "grpMatchingCondition";
            this.grpMatchingCondition.Size = new Size(440, 0x40);
            this.grpMatchingCondition.TabIndex = 2;
            this.grpMatchingCondition.TabStop = false;
            this.grpMatchingCondition.Text = "検索にヒットするための条件";
            this.radBackward.AutoSize = true;
            this.radBackward.Location = new Point(320, 0x1c);
            this.radBackward.Name = "radBackward";
            this.radBackward.Size = new Size(0x47, 0x10);
            this.radBackward.TabIndex = 2;
            this.radBackward.TabStop = true;
            this.radBackward.Text = "後方一致";
            this.radBackward.UseVisualStyleBackColor = true;
            this.radPartial.AutoSize = true;
            this.radPartial.Location = new Point(0xb3, 0x1c);
            this.radPartial.Name = "radPartial";
            this.radPartial.Size = new Size(0x47, 0x10);
            this.radPartial.TabIndex = 1;
            this.radPartial.TabStop = true;
            this.radPartial.Text = "部分一致";
            this.radPartial.UseVisualStyleBackColor = true;
            this.radForward.AutoSize = true;
            this.radForward.Location = new Point(0x20, 0x1c);
            this.radForward.Name = "radForward";
            this.radForward.Size = new Size(0x47, 0x10);
            this.radForward.TabIndex = 0;
            this.radForward.TabStop = true;
            this.radForward.Text = "前方一致";
            this.radForward.UseVisualStyleBackColor = true;
            this.grpPaging.Controls.Add(this.lblPageSize);
            this.grpPaging.Controls.Add(this.numPageSize);
            this.grpPaging.Location = new Point(12, 0xde);
            this.grpPaging.Name = "grpPaging";
            this.grpPaging.Size = new Size(440, 0x40);
            this.grpPaging.TabIndex = 3;
            this.grpPaging.TabStop = false;
            this.grpPaging.Text = "1ページの表示件数";
            this.lblPageSize.AutoSize = true;
            this.lblPageSize.Location = new Point(110, 0x1f);
            this.lblPageSize.Name = "lblPageSize";
            this.lblPageSize.Size = new Size(0x11, 12);
            this.lblPageSize.TabIndex = 1;
            this.lblPageSize.Text = "件";
            this.numPageSize.Location = new Point(0x20, 0x1d);
            int[] bits = new int[4];
            bits[0] = 0xea60;
            this.numPageSize.Maximum = new decimal(bits);
            int[] numArray2 = new int[4];
            numArray2[0] = 1;
            this.numPageSize.Minimum = new decimal(numArray2);
            this.numPageSize.Name = "numPageSize";
            this.numPageSize.Size = new Size(0x48, 0x13);
            this.numPageSize.TabIndex = 0;
            int[] numArray3 = new int[4];
            numArray3[0] = 100;
            this.numPageSize.Value = new decimal(numArray3);
            this.btnApply.Location = new Point(0x5d, 0x147);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new Size(0x4b, 0x17);
            this.btnApply.TabIndex = 6;
            this.btnApply.Text = "適用(A)";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Visible = false;
            this.btnCancel.DialogResult = DialogResult.Cancel;
            this.btnCancel.Location = new Point(0x179, 0x147);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new Size(0x4b, 0x17);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "キャンセル";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnOK.Location = new Point(0x128, 0x147);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new Size(0x4b, 0x17);
            this.btnOK.TabIndex = 4;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnInitialize.Location = new Point(12, 0x147);
            this.btnInitialize.Name = "btnInitialize";
            this.btnInitialize.Size = new Size(0x4b, 0x17);
            this.btnInitialize.TabIndex = 0x10;
            this.btnInitialize.Text = "初期設定";
            this.btnInitialize.UseVisualStyleBackColor = true;
            this.btnInitialize.Visible = false;
            base.AutoScaleDimensions = new SizeF(6f, 12f);
            base.AutoScaleMode = AutoScaleMode.Font;
            base.CancelButton = this.btnCancel;
            base.ClientSize = new Size(0x1d0, 0x16a);
            base.Controls.Add(this.btnInitialize);
            base.Controls.Add(this.btnApply);
            base.Controls.Add(this.btnCancel);
            base.Controls.Add(this.btnOK);
            base.Controls.Add(this.grpPaging);
            base.Controls.Add(this.grpMatchingCondition);
            base.Controls.Add(this.grpLogicalCondition);
            base.Controls.Add(this.grpTargetField);
            base.FormBorderStyle = FormBorderStyle.FixedDialog;
            base.MaximizeBox = false;
            base.MinimizeBox = false;
            base.Name = "FindSettingView";
            base.ShowIcon = false;
            base.ShowInTaskbar = false;
            base.StartPosition = FormStartPosition.CenterParent;
            this.Text = "検索条件設定";
            this.grpTargetField.ResumeLayout(false);
            this.grpTargetField.PerformLayout();
            this.grpLogicalCondition.ResumeLayout(false);
            this.grpLogicalCondition.PerformLayout();
            this.grpMatchingCondition.ResumeLayout(false);
            this.grpMatchingCondition.PerformLayout();
            this.grpPaging.ResumeLayout(false);
            this.grpPaging.PerformLayout();
            this.numPageSize.EndInit();
            base.ResumeLayout(false);
        }

        private void UpdateFindSetting()
        {
            UserSettings.FindSettings find = new UserSettings.FindSettings {
                Target = !this.radYomi.Checked ? UserSettings.FindSettings.TargetField.Text : UserSettings.FindSettings.TargetField.Yomi,
                Logic = !this.radOr.Checked ? UserSettings.FindSettings.LogicalCondition.And : UserSettings.FindSettings.LogicalCondition.Or,
                Match = this.radForward.Checked ? UserSettings.FindSettings.MatchingCondition.Forward : (this.radBackward.Checked ? UserSettings.FindSettings.MatchingCondition.Backward : UserSettings.FindSettings.MatchingCondition.Partial),
                PageSize = (int) this.numPageSize.Value
            };
            this._controller.UpdateFindSetting(find);
        }
    }
}

