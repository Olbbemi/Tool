namespace MakeParser
{
    partial class SystemToolForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SystemToolForm));
            this.SAddButton = new System.Windows.Forms.Button();
            this.SSaveButton = new System.Windows.Forms.Button();
            this.SystemPanel = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.SFileText = new System.Windows.Forms.TextBox();
            this.SystemPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // SAddButton
            // 
            this.SAddButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.SAddButton.Location = new System.Drawing.Point(3, 3);
            this.SAddButton.Name = "SAddButton";
            this.SAddButton.Size = new System.Drawing.Size(45, 20);
            this.SAddButton.TabIndex = 0;
            this.SAddButton.Text = "Add";
            this.SAddButton.UseVisualStyleBackColor = true;
            this.SAddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // SSaveButton
            // 
            this.SSaveButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.SSaveButton.Location = new System.Drawing.Point(3, 590);
            this.SSaveButton.Name = "SSaveButton";
            this.SSaveButton.Size = new System.Drawing.Size(47, 28);
            this.SSaveButton.TabIndex = 3;
            this.SSaveButton.Text = "Save";
            this.SSaveButton.UseVisualStyleBackColor = true;
            this.SSaveButton.Click += new System.EventHandler(this.SSaveButton_Click);
            // 
            // SystemPanel
            // 
            this.SystemPanel.Controls.Add(this.SAddButton);
            this.SystemPanel.Controls.Add(this.SSaveButton);
            this.SystemPanel.Location = new System.Drawing.Point(3, 3);
            this.SystemPanel.Name = "SystemPanel";
            this.SystemPanel.Size = new System.Drawing.Size(63, 621);
            this.SystemPanel.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label1.Location = new System.Drawing.Point(75, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(94, 15);
            this.label1.TabIndex = 4;
            this.label1.Text = "Input Filename :";
            // 
            // SFileText
            // 
            this.SFileText.Location = new System.Drawing.Point(173, 4);
            this.SFileText.Name = "SFileText";
            this.SFileText.Size = new System.Drawing.Size(320, 21);
            this.SFileText.TabIndex = 5;
            // 
            // SystemToolForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1295, 636);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.SFileText);
            this.Controls.Add(this.SystemPanel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "SystemToolForm";
            this.Text = "SystemToolForm";
            this.Load += new System.EventHandler(this.SystemToolForm_Load);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.SystemToolForm_MouseClick);
            this.SystemPanel.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button SAddButton;
        private System.Windows.Forms.Button SSaveButton;
        private System.Windows.Forms.Panel SystemPanel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox SFileText;
    }
}