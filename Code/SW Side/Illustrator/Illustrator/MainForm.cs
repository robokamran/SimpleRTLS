using MathNet.Numerics;
using MathNet.Numerics.Data.Text;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Double;
using MetroFramework;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using Newtonsoft.Json;
using ScottPlot;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Zeroit.Framework.MaterialDesign.Controls;

namespace Illustrator
{
    public partial class MainForm : ZeroitMaterialForm
    {
        IMqttClient mqttClient = new MqttFactory().CreateMqttClient();
        IMqttClientOptions options = new MqttClientOptionsBuilder().WithTcpServer("broker.mqttdashboard.com").Build();
        Matrix<double> beacons = Matrix.Build.Dense(3, 2, 0);

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            ProfX.ts = ProfY.ts = ProfH.ts = 0.001 * timer.Interval;

            formsPlot.plt.Axis(0, 10, 0, 10);
            //formsPlot.plt.Ticks(displayTicksX: false, displayTicksY: false);
            //formsPlot.plt.Frame(drawFrame: false);
            //formsPlot.plt.Grid(false);
            formsPlot.plt.Style(Style.Default);

            timer.Start();
        }

        public struct NavigationData
        {
            public double D1, D2, D3;
            public double Heading;
            public double X, Y;
        }

        NavigationData NavData = new NavigationData();

        void Handler(MqttApplicationMessageReceivedEventArgs e)
        {
            try
            {
                // extract raw data
                var payload = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);
                NavData = JsonConvert.DeserializeObject<NavigationData>(payload);
                // compute location
                var A = beacons.Multiply(-2).Append(Matrix.Build.Dense(3, 1, 1));
                var R = Vector.Build.DenseOfArray(new double[] { NavData.D1, NavData.D2, NavData.D3 });
                var Y = R.PointwisePower(2) - beacons.Column(0).PointwisePower(2) - beacons.Column(1).PointwisePower(2);
                var X = A.PseudoInverse() * Y;
                // store computed location
                NavData.X = X[0];
                NavData.Y = X[1];
                // show distances
                Invoke((Action)(() => zeroitLabelAlterDistances.Text =
                "D1: " + Math.Round(NavData.D1, 1) +
                " - D2: " + Math.Round(NavData.D2, 1) +
                " - D3: " + Math.Round(NavData.D3, 1)));
            }
            catch (Exception ex)
            {
                MetroMessageBox.Show(this, ex.Message, "Generral Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        Queue<double> QueueX = new Queue<double>();
        Queue<double> QueueY = new Queue<double>();
        Profile ProfX = new Profile(50);
        Profile ProfY = new Profile(50);
        Profile ProfH = new Profile(200);

        private void Timer_Tick(object sender, EventArgs e)
        {
            zeroitSlidingScale.Value = ProfH.Step(NavData.Heading);
            QueueX.Enqueue(ProfX.Step(NavData.X));
            QueueY.Enqueue(ProfY.Step(NavData.Y));
            if (QueueX.Count > 100)
            {
                QueueX.Dequeue();
                QueueY.Dequeue();
            }
            formsPlot.plt.Clear();
            formsPlot.plt.PlotScatter(QueueX.ToArray(), QueueY.ToArray(), markerShape: MarkerShape.none);
            formsPlot.plt.PlotPoint(ProfX.x1, ProfY.x1);
            formsPlot.plt.PlotPoint(beacons[0, 0], beacons[0, 1], color: Color.Green, markerShape: MarkerShape.filledSquare);
            formsPlot.plt.PlotPoint(beacons[1, 0], beacons[1, 1], color: Color.Green, markerShape: MarkerShape.filledSquare);
            formsPlot.plt.PlotPoint(beacons[2, 0], beacons[2, 1], color: Color.Green, markerShape: MarkerShape.filledSquare);
            formsPlot.plt.PlotText("A1", beacons[0, 0], beacons[0, 1], color: Color.Magenta);
            formsPlot.plt.PlotText("A2", beacons[1, 0], beacons[1, 1], color: Color.Magenta);
            formsPlot.plt.PlotText("A3", beacons[2, 0], beacons[2, 1], color: Color.Magenta);
            formsPlot.Render();
        }

        private void ZeroitMaterialRaisedButtonReset_Click(object sender, EventArgs e)
        {
            beacons = DelimitedReader.Read<double>(new StringReader(zeroitCodeTextBoxLocations.Text));
            var xSpan = beacons.Column(0).Max() - beacons.Column(0).Min();
            var ySpan = beacons.Column(1).Max() - beacons.Column(1).Min();
            var x1 = beacons.Column(0).Min() - xSpan * 0.2;
            var x2 = beacons.Column(0).Max() + xSpan * 0.2;
            var y1 = beacons.Column(1).Min() - ySpan * 0.2;
            var y2 = beacons.Column(1).Max() + ySpan * 0.2;
            formsPlot.plt.Axis(x1, x2, y1, y2);

            // define location
            var pos = Vector.Build.DenseOfArray(new double[] { 2, 3 });
            NavData.D1 = Distance.Euclidean(pos, beacons.Row(0));
            NavData.D2 = Distance.Euclidean(pos, beacons.Row(1));
            NavData.D3 = Distance.Euclidean(pos, beacons.Row(2));
            // compute location
            var A = beacons.Multiply(-2).Append(Matrix.Build.Dense(3, 1, 1));
            var R = Vector.Build.DenseOfArray(new double[] { NavData.D1, NavData.D2, NavData.D3 });
            var Y = R.PointwisePower(2) - beacons.Column(0).PointwisePower(2) - beacons.Column(1).PointwisePower(2);
            var X = A.PseudoInverse() * Y;
            // store computed location
            NavData.X = X[0];
            NavData.Y = X[1];
        }

        private void ZeroitToggleXConnection_ToggledChanged()
        {
            if (zeroitToggleXConnection.Toggled)
            {
                ZeroitMaterialRaisedButtonReset_Click(null, null);
                //serialPort.Open();
                mqttClient.ConnectAsync(options).ContinueWith(task =>
                    {
                        MetroMessageBox.Show(this,
                        task.IsFaulted ? task.Exception.Message : "Connected",
                        "Connection State", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        mqttClient.UseApplicationMessageReceivedHandler(Handler);
                        mqttClient.SubscribeAsync("KNTUMasters2020");
                    });
            }
            else
            {
                //serialPort.Close();
                mqttClient.ApplicationMessageReceivedHandler = null;
                mqttClient.UnsubscribeAsync("KNTUMasters2020");
                mqttClient.DisconnectAsync()
                    .ContinueWith(
                    (task => MetroMessageBox.Show(this, "Disonnected", "Connection State",
                    MessageBoxButtons.OK, MessageBoxIcon.Information)));
            }
        }

        private void SerialPort_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            try
            {
                // extract raw data
                var payload = serialPort.ReadLine();
                NavData = JsonConvert.DeserializeObject<NavigationData>(payload);
                // compute location
                var A = beacons.Multiply(-2).Append(Matrix.Build.Dense(3, 1, 1));
                var R = Vector.Build.DenseOfArray(new double[] { NavData.D1, NavData.D2, NavData.D3 });
                var Y = R.PointwisePower(2) - beacons.Column(0).PointwisePower(2) - beacons.Column(1).PointwisePower(2);
                var X = A.PseudoInverse() * Y;
                // store computed location
                NavData.X = X[0];
                NavData.Y = X[1];
                // show distances
                Invoke((Action)(() => zeroitLabelAlterDistances.Text =
                "D1: " + Math.Round(NavData.D1, 1) +
                " - D2: " + Math.Round(NavData.D2, 1) +
                " - D3: " + Math.Round(NavData.D3, 1)));
            }
            catch { }
        }
    }

    public class Profile
    {
        public double x1, x2, x2max, x3max, ts;

        public Profile(double X3Max = 1, double X2Max = double.MaxValue, double Ts = 1)
        {
            x2max = X2Max;
            x3max = X3Max;
            ts = Ts;
            x1 = x2 = 0;
        }

        private double fhan(double x1, double x2, double r, double h)
        {
            double d = r * h * h;
            double a0 = h * x2;
            double y = x1 + a0;
            double a1 = Math.Sqrt(d * (d + 8 * Math.Abs(y)));
            double a2 = a0 + Math.Sign(y) * (a1 - d) / 2;
            double sy = (Math.Sign(y + d) - Math.Sign(y - d)) / 2;
            double a = (a0 + y - a2) * sy + a2;
            double sa = (Math.Sign(a + d) - Math.Sign(a - d)) / 2;
            double f = -r * (a / d - Math.Sign(a)) * sa - r * Math.Sign(a);
            return f;
        }

        public double Step(double x1r)
        {
            double fv = fhan(x1 - x1r, x2, x3max, ts);
            x1 += ts * x2;
            x2 += ts * fv;
            x2 = Math.Min(Math.Max(x2, -x2max), +x2max);
            return x1;
        }
    }
}
