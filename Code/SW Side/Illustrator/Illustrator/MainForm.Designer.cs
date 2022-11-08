namespace Illustrator
{
    partial class MainForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.formsPlot = new ScottPlot.FormsPlot();
            this.zeroitSlidingScale = new Zeroit.Framework.MiscControls.ZeroitSlidingScale();
            this.zeroitMaterialRaisedButtonReset = new Zeroit.Framework.MaterialDesign.Controls.ZeroitMaterialRaisedButton();
            this.zeroitToggleXConnection = new Zeroit.Framework.MiscControls.ZeroitToggleX();
            this.zeroitCodeTextBoxLocations = new Zeroit.Framework.MiscControls.FastControls.ZeroitCodeTextBox();
            this.zeroitLabelAlterDistances = new Zeroit.Framework.MiscControls.ZeroitLabelAlter();
            this.serialPort = new System.IO.Ports.SerialPort(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.zeroitCodeTextBoxLocations)).BeginInit();
            this.SuspendLayout();
            // 
            // timer
            // 
            this.timer.Interval = 20;
            this.timer.Tick += new System.EventHandler(this.Timer_Tick);
            // 
            // formsPlot
            // 
            this.formsPlot.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.formsPlot.Location = new System.Drawing.Point(12, 121);
            this.formsPlot.Margin = new System.Windows.Forms.Padding(4);
            this.formsPlot.Name = "formsPlot";
            this.formsPlot.Size = new System.Drawing.Size(500, 339);
            this.formsPlot.TabIndex = 0;
            // 
            // zeroitSlidingScale
            // 
            this.zeroitSlidingScale.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.zeroitSlidingScale.BackColor = System.Drawing.Color.White;
            this.zeroitSlidingScale.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.zeroitSlidingScale.Control = null;
            this.zeroitSlidingScale.ForeColor = System.Drawing.Color.Black;
            this.zeroitSlidingScale.Location = new System.Drawing.Point(12, 82);
            this.zeroitSlidingScale.Name = "zeroitSlidingScale";
            this.zeroitSlidingScale.Size = new System.Drawing.Size(500, 33);
            this.zeroitSlidingScale.TabIndex = 4;
            // 
            // zeroitMaterialRaisedButtonReset
            // 
            this.zeroitMaterialRaisedButtonReset.AllowTransparency = true;
            this.zeroitMaterialRaisedButtonReset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.zeroitMaterialRaisedButtonReset.Depth = 0;
            this.zeroitMaterialRaisedButtonReset.Icon = null;
            this.zeroitMaterialRaisedButtonReset.Location = new System.Drawing.Point(519, 424);
            this.zeroitMaterialRaisedButtonReset.MouseState = Zeroit.Framework.MaterialDesign.MouseState.HOVER;
            this.zeroitMaterialRaisedButtonReset.Name = "zeroitMaterialRaisedButtonReset";
            this.zeroitMaterialRaisedButtonReset.Primary = true;
            this.zeroitMaterialRaisedButtonReset.Size = new System.Drawing.Size(107, 36);
            this.zeroitMaterialRaisedButtonReset.TabIndex = 9;
            this.zeroitMaterialRaisedButtonReset.Text = "Reset Scale";
            this.zeroitMaterialRaisedButtonReset.Click += new System.EventHandler(this.ZeroitMaterialRaisedButtonReset_Click);
            // 
            // zeroitToggleXConnection
            // 
            this.zeroitToggleXConnection.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.zeroitToggleXConnection.BackColor = System.Drawing.Color.Transparent;
            this.zeroitToggleXConnection.ColorBigBackground = System.Drawing.Color.FromArgb(((int)(((byte)(32)))), ((int)(((byte)(41)))), ((int)(((byte)(50)))));
            this.zeroitToggleXConnection.ColorBigBackgroundBorder = System.Drawing.Color.Black;
            this.zeroitToggleXConnection.ColorBigBackgroundToggled = System.Drawing.Color.FromArgb(((int)(((byte)(32)))), ((int)(((byte)(41)))), ((int)(((byte)(50)))));
            this.zeroitToggleXConnection.ColorSmallBackground = System.Drawing.Color.FromArgb(((int)(((byte)(66)))), ((int)(((byte)(76)))), ((int)(((byte)(85)))));
            this.zeroitToggleXConnection.ColorSmallBackgroundBorder = System.Drawing.Color.Black;
            this.zeroitToggleXConnection.ColorSmallBackgroundToggled = System.Drawing.Color.FromArgb(((int)(((byte)(181)))), ((int)(((byte)(41)))), ((int)(((byte)(42)))));
            this.zeroitToggleXConnection.Location = new System.Drawing.Point(827, 82);
            this.zeroitToggleXConnection.Name = "zeroitToggleXConnection";
            this.zeroitToggleXConnection.Size = new System.Drawing.Size(76, 33);
            this.zeroitToggleXConnection.TabIndex = 13;
            this.zeroitToggleXConnection.Text = "zeroitToggleX1";
            this.zeroitToggleXConnection.Toggled = false;
            this.zeroitToggleXConnection.Type = Zeroit.Framework.MiscControls.ZeroitToggleX.TypeOfToggle.CheckMark;
            this.zeroitToggleXConnection.ToggledChanged += new Zeroit.Framework.MiscControls.ZeroitToggleX.ToggledChangedEventHandler(this.ZeroitToggleXConnection_ToggledChanged);
            // 
            // zeroitCodeTextBoxLocations
            // 
            this.zeroitCodeTextBoxLocations.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.zeroitCodeTextBoxLocations.AutoCompleteBracketsList = new char[] {
        '(',
        ')',
        '{',
        '}',
        '[',
        ']',
        '\"',
        '\"',
        '\'',
        '\''};
            this.zeroitCodeTextBoxLocations.AutoScrollMinSize = new System.Drawing.Size(67, 42);
            this.zeroitCodeTextBoxLocations.BackBrush = null;
            this.zeroitCodeTextBoxLocations.CharHeight = 14;
            this.zeroitCodeTextBoxLocations.CharWidth = 8;
            this.zeroitCodeTextBoxLocations.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.zeroitCodeTextBoxLocations.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
            this.zeroitCodeTextBoxLocations.Font = new System.Drawing.Font("Courier New", 9.75F);
            this.zeroitCodeTextBoxLocations.IsReplaceMode = false;
            this.zeroitCodeTextBoxLocations.Location = new System.Drawing.Point(518, 121);
            this.zeroitCodeTextBoxLocations.Name = "zeroitCodeTextBoxLocations";
            this.zeroitCodeTextBoxLocations.Paddings = new System.Windows.Forms.Padding(0);
            this.zeroitCodeTextBoxLocations.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
            this.zeroitCodeTextBoxLocations.ServiceColors = ((Zeroit.Framework.MiscControls.FastControls.ServiceColors)(resources.GetObject("zeroitCodeTextBoxLocations.ServiceColors")));
            this.zeroitCodeTextBoxLocations.Size = new System.Drawing.Size(385, 297);
            this.zeroitCodeTextBoxLocations.TabIndex = 14;
            this.zeroitCodeTextBoxLocations.Text = "0 3.2\r\n7.2 4\r\n4.8 0";
            this.zeroitCodeTextBoxLocations.Zoom = 100;
            // 
            // zeroitLabelAlterDistances
            // 
            this.zeroitLabelAlterDistances.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.zeroitLabelAlterDistances.AutoSize = true;
            this.zeroitLabelAlterDistances.BackColor = System.Drawing.Color.Transparent;
            this.zeroitLabelAlterDistances.Font = new System.Drawing.Font("Verdana", 8F);
            this.zeroitLabelAlterDistances.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
            this.zeroitLabelAlterDistances.Location = new System.Drawing.Point(516, 95);
            this.zeroitLabelAlterDistances.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.zeroitLabelAlterDistances.Name = "zeroitLabelAlterDistances";
            this.zeroitLabelAlterDistances.Size = new System.Drawing.Size(62, 13);
            this.zeroitLabelAlterDistances.TabIndex = 17;
            this.zeroitLabelAlterDistances.Text = "Distances";
            // 
            // serialPort
            // 
            this.serialPort.BaudRate = 115200;
            this.serialPort.PortName = "COM21";
            this.serialPort.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.SerialPort_DataReceived);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(915, 472);
            this.Controls.Add(this.zeroitLabelAlterDistances);
            this.Controls.Add(this.zeroitCodeTextBoxLocations);
            this.Controls.Add(this.zeroitToggleXConnection);
            this.Controls.Add(this.zeroitMaterialRaisedButtonReset);
            this.Controls.Add(this.zeroitSlidingScale);
            this.Controls.Add(this.formsPlot);
            this.MinimumSize = new System.Drawing.Size(126, 39);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Illustrator";
            this.TransparencyKey = System.Drawing.Color.Fuchsia;
            this.Load += new System.EventHandler(this.MainForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.zeroitCodeTextBoxLocations)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Timer timer;
        private ScottPlot.FormsPlot formsPlot;
        private Zeroit.Framework.MiscControls.ZeroitSlidingScale zeroitSlidingScale;
        private Zeroit.Framework.MaterialDesign.Controls.ZeroitMaterialRaisedButton zeroitMaterialRaisedButtonReset;
        private Zeroit.Framework.MiscControls.ZeroitToggleX zeroitToggleXConnection;
        private Zeroit.Framework.MiscControls.FastControls.ZeroitCodeTextBox zeroitCodeTextBoxLocations;
        private Zeroit.Framework.MiscControls.ZeroitLabelAlter zeroitLabelAlterDistances;
        private System.IO.Ports.SerialPort serialPort;
    }
}

