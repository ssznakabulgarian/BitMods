using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace DesktopClient
{
    public partial class LoadingScreen : Form
    {
        private readonly Timer mTimer = new Timer();

        public LoadingScreen()
        {
            InitializeComponent();
            mTimer.Interval = 2000;
            mTimer.Tick += MTimer_Tick;
            mTimer.Start();
        }

        private void MTimer_Tick(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
        }
    }
}
