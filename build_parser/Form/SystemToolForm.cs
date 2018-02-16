using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;

namespace MakeParser
{
    public partial class SystemToolForm : Form
    {
        bool check;
        int yPos, yCnt, xCnt, buttonCnt;
        Queue<int> Xqueue;

        List<TextBox> Tlist;
        TextBox ScopeBox, FtextBox, StextBox;
        List<Button> Blist;
        Button Pbt, Mbt;

        List<Label> Llist;
        Label mainScope;

        public SystemToolForm()
        {
            InitializeComponent();
            check = false;
            yPos = 30; yCnt = 0; xCnt = 0; buttonCnt = 0;
        }

        private void SSaveButton_Click(object sender, EventArgs e)
        {
            bool confirm = NameCheck();

            if (confirm)
                return;

            FileStream output = new FileStream(SFileText.Text + ".txt", FileMode.Append, FileAccess.Write);
            StreamWriter Wstream = new StreamWriter(output, System.Text.Encoding.Unicode);

            foreach(TextBox temp in Tlist)
            {
                if(temp.Name == "ScopeText")
                {
                    for (int i = 0; i < Xqueue.Peek(); i++)
                        Wstream.Write("\t");
                    Wstream.WriteLine(temp.Text);

                    for (int i = 0; i < Xqueue.Peek(); i++)
                        Wstream.Write("\t");

                    Wstream.WriteLine(">");
                    Xqueue.Dequeue();
                }
                else if(temp.Name == "FDataText")
                {
                    for (int i = 0; i < Xqueue.Peek(); i++)
                        Wstream.Write("\t");

                    Wstream.Write(temp.Text + "\t=");
                    Xqueue.Dequeue();
                }
                else
                    Wstream.Write("\t" + temp.Text + ";" + Environment.NewLine);
            
                // 생성된 컴포넌트 삭제
                this.Controls.Remove(temp);
                temp.Dispose();
            }

            foreach(Label temp in Llist)
            {
                this.Controls.Remove(temp);
                temp.Dispose();
            }

            foreach(Button temp in Blist)
            {
                this.Controls.Remove(temp);
                temp.Dispose();
            }

            Blist = null;    Tlist = null;
            Llist = null;    Xqueue = null;

            Wstream.Write(Environment.NewLine);
            check = false;
            yPos = 30; xCnt = 0; buttonCnt = 0; yCnt = 0;
            Wstream.Flush(); Wstream.Close();
            output.Close();
        }

        private void SystemToolForm_Load(object sender, EventArgs e)
        {
            this.KeyPreview = true;
            this.Text = "System Parser";
            this.Size = new System.Drawing.Size(1310, 675);
            this.Location = new Point(Screen.PrimaryScreen.Bounds.Width / 2 - this.Width / 2, Screen.PrimaryScreen.Bounds.Height / 2 - this.Height / 2);
            SystemPanel.Dock = DockStyle.Left;
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            if (check == false)
            {
                Blist = new List<Button>();
                Tlist = new List<TextBox>();
                Llist = new List<Label>();
                Xqueue = new Queue<int>();

                check = true;
                mainScope = new Label();
                mainScope.Name = "";
                mainScope.Text = "Scope :";
                mainScope.Location = new Point(75, 37);
                mainScope.Size = new Size(50, 12);

                ScopeBox = new TextBox();
                ScopeBox.Name = "ScopeText";
                ScopeBox.Location = new Point(125, 33);
                ScopeBox.Size = new Size(100, 21);
                
                this.Controls.Add(mainScope);
                this.Controls.Add(ScopeBox);
                Llist.Add(mainScope);
                Tlist.Add(ScopeBox);
                Xqueue.Enqueue(xCnt);
            }
        }

        private void SystemToolForm_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button.Equals(MouseButtons.Right))
            {
                if (check == true && yCnt < 19)
                {
                    yPos += 30; yCnt++;
                    FtextBox = new TextBox();
                    FtextBox.Name = "FDataText";
                    FtextBox.Location = new Point(80 + xCnt * 200, yPos);
                    FtextBox.Size = new Size(75, 21);
                    this.Controls.Add(FtextBox);

                    StextBox = new TextBox();
                    StextBox.Name = "SDataText";
                    StextBox.Location = new Point(165 + xCnt * 200, yPos);
                    StextBox.Size = new Size(60, 21);
                    this.Controls.Add(StextBox);

                    Tlist.Add(FtextBox);
                    Tlist.Add(StextBox);
                    Xqueue.Enqueue(xCnt);

                    Pbt = new Button();
                    Pbt.Text = "+";
                    Pbt.Location = new Point(230 + xCnt * 200, yPos);
                    Pbt.Size = new Size(20, 22);
                    Pbt.Click += new EventHandler(Pbutton_click);
                    this.Controls.Add(Pbt);

                    Mbt = new Button();
                    Mbt.Text = "-";
                    Mbt.Location = new Point(250 + xCnt * 200, yPos);
                    Mbt.Size = new Size(20, 22);
                    Mbt.Click += new EventHandler(Mbutton_click);
                    this.Controls.Add(Mbt);

                    Blist.Add(Pbt); Blist.Add(Mbt);

                    if (buttonCnt >= 2)
                        Blist[buttonCnt - 2].Enabled = Blist[buttonCnt - 1].Enabled = false;

                    buttonCnt += 2;
                }
            }
        }

        private void Pbutton_click(object sender, EventArgs e)
        {
            Control ctl = sender as Control;
            if (ctl != null)
            {
                xCnt++;
                mainScope = new Label();
                mainScope.Name = "";
                mainScope.Text = "Scope :";
                mainScope.Location = new Point(80 + xCnt * 200 - 5, yPos + 5);
                mainScope.Size = new Size(50, 12);

                ScopeBox = new TextBox();
                ScopeBox.Name = "ScopeText";
                ScopeBox.Location = new Point(80 + xCnt * 200 + 45, yPos);
                ScopeBox.Size = new Size(100, 21);

                this.Controls.Add(mainScope);
                this.Controls.Add(ScopeBox);
                Llist.Add(mainScope);
                Tlist.Add(ScopeBox);
                Xqueue.Enqueue(xCnt);
            }
        }

        private void Mbutton_click(object sender, EventArgs e)
        {
            Control ctl = sender as Control;
            if (ctl != null && xCnt != 0)
                xCnt--;
        }

        private bool NameCheck()
        {
            if (string.IsNullOrWhiteSpace(SFileText.Text) == true)
            {
                MessageBox.Show("파일이름을 입력하세요.");
                return true;
            }
            else
                return false;
        }
    }
}