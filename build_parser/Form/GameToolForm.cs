using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace MakeParser
{
    public partial class GameToolForm : Form
    {
        DataGridView GridView;
        List<TextBox> textList;
        int index, num, yPos, boxCnt;
       
        public GameToolForm()
        {
            InitializeComponent();
            GridView = new DataGridView();
            textList = new List<TextBox>();
            index = 0;  num = 1;    yPos = 12;  boxCnt = 0;
        }

        private void GameToolForm_Load(object sender, EventArgs e)
        {
            this.MaximizeBox = false;
            this.Text = "Game Parser";
            this.Size = new System.Drawing.Size(1295, 510);
            this.Location = new Point(Screen.PrimaryScreen.Bounds.Width / 2 - this.Width / 2, Screen.PrimaryScreen.Bounds.Height / 2 - this.Height / 2);
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            FileStream output = new FileStream(GFileText.Text + ".txt", FileMode.Append, FileAccess.Write);
            StreamWriter Wstream = new StreamWriter(output, System.Text.Encoding.Unicode);

            //GridView.RowCount 통해서 현재 열의 개수 알 수 있음, 행의 개수는 boxCnt
            int row = GridView.RowCount - 1, col = boxCnt;

            for(int i = 0; i < row; i++)
            {
                Wstream.Write("|");
                for (int j = 0; j < col; j++)
                    Wstream.Write(string.Format("{0,20}", GridView.Rows[i].Cells[j].FormattedValue.ToString() + " |"));
             
                Wstream.Write(Environment.NewLine);
            }

            // 모든 텍스트 삭제, 파일이름 초기화, 데이터 그리드 삭제
            foreach (TextBox temp in textList)
            {
                this.Controls.Remove(temp);
                temp.Dispose();
            }

            GFileText.Text = "";
            this.Controls.Remove(GridView);
            GridView.Dispose();

            Wstream.Flush(); Wstream.Close();
            output.Close();
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            if (boxCnt > 10)
                return;

            num++; yPos += 36; boxCnt++;
           
            TextBox tbox = new TextBox();
            tbox.Location = new Point(12, yPos);
            tbox.Name = "TBox" + num.ToString();
            tbox.Size = new Size(44, 21);

            textList.Add(tbox);

            this.Controls.Add(tbox);
        }

        private void CreateButton_Click(object sender, EventArgs e)
        {
            int cnt = 0;

            if (string.IsNullOrWhiteSpace(GFileText.Text))
            {
                MessageBox.Show("파일이름을 입력하세요.");
                return;
            }

            FileOperator();

            if (boxCnt == 0)
                return;
            
            foreach (TextBox temp in textList)
            {
                if(string.IsNullOrWhiteSpace(temp.Text) == true)
                {
                    MessageBox.Show((cnt + 1).ToString() + "번째 문자를 입력해주세요");
                    return;
                }

                cnt++;
            }
            
            GridView.Location = new Point(125, 35);
            GridView.Size = new Size(1145, 425);
            GridView.ColumnCount = boxCnt;

            foreach (TextBox temp in textList)
            {
                GridView.Columns[index++].Name = temp.Text;
                temp.Text = "";
            }
            
            this.Controls.Add(GridView);
        }

        private void FileOperator()
        {
            FileStream output = new FileStream(GFileText.Text + ".txt", FileMode.Create, FileAccess.Write);
            StreamWriter Wstream = new StreamWriter(output, System.Text.Encoding.Unicode);

            Wstream.Write("|");
            foreach (TextBox temp in textList)
                Wstream.Write(string.Format("{0,20}", temp.Text + " |"));
            Wstream.Write(Environment.NewLine);

            Wstream.Flush();    Wstream.Close();
            output.Close();
        }
    }
}