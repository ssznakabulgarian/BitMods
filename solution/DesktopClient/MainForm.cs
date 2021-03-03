using Core;
using DesktopClient.BotControl;
using DesktopClient.WebServer;
using Microsoft.Web.WebView2.WinForms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DesktopClient
{
    public partial class MainForm : Form
    {
        private readonly WebView2 mBrowser;
        private IDispatcher mDispatcher;

        public MainForm()
        {
            InitializeComponent();
            mBrowser = new WebView2();
            mBrowser.Dock = DockStyle.Fill;
            Controls.Add(mBrowser);
        }

        private void MBrowser_CoreWebView2InitializationCompleted(object sender, Microsoft.Web.WebView2.Core.CoreWebView2InitializationCompletedEventArgs e)
        {
            LocalServer.Start(mDispatcher, (string error) =>
            {
                if (error != null)
                {
                    MessageBox.Show(error, "Error", MessageBoxButtons.OK);
                    Close();
                }
                else
                {
                    mBrowser.CoreWebView2.Navigate("http://localhost:9999/index.html");
                }
            });
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            mDispatcher = new DispatcherImpl(this);
            mBrowser.CoreWebView2InitializationCompleted += MBrowser_CoreWebView2InitializationCompleted;
            mBrowser.EnsureCoreWebView2Async();

            //TEST COM CONNECTION. TO BE REMOVED
            string[] ports = SerialPort.GetPortNames();
            if (ports.Length > 0)
            {
                BotMain bot = new BotMain(mDispatcher, "test");
                ComConnection con = new ComConnection(mDispatcher);
                con.SetPortName(ports[0]);
                bot.SetConnection(con);
                bot.OnModAdded += Bot_OnModAdded;
                bot.IsAvailableChanged += Bot_IsAvailableChanged;
                bot.RunningChanged += Bot_RunningChanged;
                bot.Start();
            }
        }

        private void Bot_RunningChanged(BotMain sender, bool running)
        {
            Debug.WriteLine("Bot_IsAvailableChanged");
        }

        private void Bot_IsAvailableChanged(ControlCore.IBot sender, bool isAvailable)
        {
            Debug.WriteLine("Bot_IsAvailableChanged");
        }

        private void Bot_OnModAdded(ControlCore.IBot sender, ControlCore.IMod mod)
        {
            Debug.WriteLine("Bot_OnModAdded");
            mod.OnModStateChanged += Mod_OnModStateChanged;
        }

        private void Mod_OnModStateChanged(ControlCore.IBot sender, ControlCore.IMod mod)
        {
            Debug.WriteLine("Mod_OnModStateChanged");
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
            LocalServer.Stop();
        }
    }
}
