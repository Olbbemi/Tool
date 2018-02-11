using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MakeParser
{
    public partial class SelectForm : Form
    {
        public SelectForm()
        {
            InitializeComponent();
        }

        private void SelectForm_Load(object sender, EventArgs e)
        {
            // SelectForm 의 위치 및 타이틀, 크기 설정
            this.MaximizeBox = false;
            this.Location = new Point(Screen.PrimaryScreen.Bounds.Width / 2 - this.Width / 2 , Screen.PrimaryScreen.Bounds.Height / 2 - this.Height / 2);
            this.Text = "Make Parser";
            this.Size = new System.Drawing.Size(245, 115);

            // SelectForm 내부 패널위치 설정
            SelectPanel.Dock = DockStyle.Fill;
        }

        private void ConfirmButton_Click(object sender, EventArgs e)
        {
            // SelectForm 숨김
            this.Hide();
            
            // 선택된 라디오 버튼에 따라 다른 폼이 생성 
            if (RSystemButton.Checked == true)
            {
                SystemToolForm systemTool = new SystemToolForm();
                systemTool.ShowDialog();
            }
            else if(RGameButton.Checked == true)
            {
                GameToolForm gameTool = new GameToolForm();
                gameTool.ShowDialog();
            }
              
            // 다른 폼이 종료되면 SelectForm 보이기
            this.Show(); // ShowDialog() 함수로 호출하면 에러발생??
        }
    }
}