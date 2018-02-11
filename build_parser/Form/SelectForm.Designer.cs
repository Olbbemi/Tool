namespace MakeParser
{
    partial class SelectForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SelectForm));
            this.SelectPanel = new System.Windows.Forms.Panel();
            this.ConfirmButton = new System.Windows.Forms.Button();
            this.RGameButton = new System.Windows.Forms.RadioButton();
            this.RSystemButton = new System.Windows.Forms.RadioButton();
            this.SelectPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // SelectPanel
            // 
            this.SelectPanel.Controls.Add(this.ConfirmButton);
            this.SelectPanel.Controls.Add(this.RGameButton);
            this.SelectPanel.Controls.Add(this.RSystemButton);
            this.SelectPanel.Location = new System.Drawing.Point(12, 12);
            this.SelectPanel.Name = "SelectPanel";
            this.SelectPanel.Size = new System.Drawing.Size(239, 76);
            this.SelectPanel.TabIndex = 0;
            // 
            // ConfirmButton
            // 
            this.ConfirmButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.ConfirmButton.Location = new System.Drawing.Point(73, 44);
            this.ConfirmButton.Name = "ConfirmButton";
            this.ConfirmButton.Size = new System.Drawing.Size(75, 23);
            this.ConfirmButton.TabIndex = 1;
            this.ConfirmButton.Text = "Confirm";
            this.ConfirmButton.UseVisualStyleBackColor = true;
            this.ConfirmButton.Click += new System.EventHandler(this.ConfirmButton_Click);
            // 
            // RGameButton
            // 
            this.RGameButton.AutoSize = true;
            this.RGameButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.RGameButton.Location = new System.Drawing.Point(22, 13);
            this.RGameButton.Name = "RGameButton";
            this.RGameButton.Size = new System.Drawing.Size(79, 19);
            this.RGameButton.TabIndex = 0;
            this.RGameButton.TabStop = true;
            this.RGameButton.Text = "GameTool";
            this.RGameButton.UseVisualStyleBackColor = true;
            // 
            // RSystemButton
            // 
            this.RSystemButton.AutoSize = true;
            this.RSystemButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.RSystemButton.Location = new System.Drawing.Point(129, 13);
            this.RSystemButton.Name = "RSystemButton";
            this.RSystemButton.Size = new System.Drawing.Size(87, 19);
            this.RSystemButton.TabIndex = 0;
            this.RSystemButton.TabStop = true;
            this.RSystemButton.Text = "SystemTool";
            this.RSystemButton.UseVisualStyleBackColor = true;
            // 
            // SelectForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(263, 100);
            this.Controls.Add(this.SelectPanel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "SelectForm";
            this.Text = "SelectForm";
            this.Load += new System.EventHandler(this.SelectForm_Load);
            this.SelectPanel.ResumeLayout(false);
            this.SelectPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel SelectPanel;
        private System.Windows.Forms.Button ConfirmButton;
        private System.Windows.Forms.RadioButton RGameButton;
        private System.Windows.Forms.RadioButton RSystemButton;
    }
}