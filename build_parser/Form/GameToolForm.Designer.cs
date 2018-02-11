namespace MakeParser
{
    partial class GameToolForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GameToolForm));
            this.GAddButton = new System.Windows.Forms.Button();
            this.GCreateButton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.GSaveButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.GFileText = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // GAddButton
            // 
            this.GAddButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.GAddButton.Location = new System.Drawing.Point(12, 12);
            this.GAddButton.Name = "GAddButton";
            this.GAddButton.Size = new System.Drawing.Size(44, 21);
            this.GAddButton.TabIndex = 0;
            this.GAddButton.Text = "Add";
            this.GAddButton.UseVisualStyleBackColor = true;
            this.GAddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // GCreateButton
            // 
            this.GCreateButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.GCreateButton.Location = new System.Drawing.Point(12, 440);
            this.GCreateButton.Name = "GCreateButton";
            this.GCreateButton.Size = new System.Drawing.Size(53, 23);
            this.GCreateButton.TabIndex = 2;
            this.GCreateButton.Text = "Create";
            this.GCreateButton.UseVisualStyleBackColor = true;
            this.GCreateButton.Click += new System.EventHandler(this.CreateButton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("맑은 고딕", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label1.Location = new System.Drawing.Point(62, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "(Max:11)";
            // 
            // GSaveButton
            // 
            this.GSaveButton.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.GSaveButton.Location = new System.Drawing.Point(71, 440);
            this.GSaveButton.Name = "GSaveButton";
            this.GSaveButton.Size = new System.Drawing.Size(45, 23);
            this.GSaveButton.TabIndex = 4;
            this.GSaveButton.Text = "Save";
            this.GSaveButton.UseVisualStyleBackColor = true;
            this.GSaveButton.Click += new System.EventHandler(this.SaveButton_Click);
            // 
            // label2
            // 
            this.label2.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.label2.Location = new System.Drawing.Point(125, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(107, 18);
            this.label2.TabIndex = 5;
            this.label2.Text = "Input Filename :";
            // 
            // GFileText
            // 
            this.GFileText.Location = new System.Drawing.Point(230, 8);
            this.GFileText.Multiline = true;
            this.GFileText.Name = "GFileText";
            this.GFileText.Size = new System.Drawing.Size(320, 20);
            this.GFileText.TabIndex = 6;
            // 
            // GameToolForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1279, 471);
            this.Controls.Add(this.GFileText);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.GSaveButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.GCreateButton);
            this.Controls.Add(this.GAddButton);
            this.Name = "GameToolForm";
            this.Text = "GameToolForm";
            this.Load += new System.EventHandler(this.GameToolForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button GAddButton;
        private System.Windows.Forms.Button GCreateButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button GSaveButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox GFileText;
    }
}