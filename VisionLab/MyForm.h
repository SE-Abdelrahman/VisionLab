#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <algorithm>

namespace VisionLab {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;

	inline cv::Mat global_src;
	inline cv::Mat global_dst;

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			ApplyModernDarkTheme();
		}

	protected:
		~MyForm()
		{
			if (components) delete components;
		}

	private:
		Panel^ topPanel;
		Panel^ sidePanel;
		Panel^ slidersPanel;
		Label^ lblTitle;
		Label^ lblInfo;
		Button^ btnLoad;
		Button^ btnSave;
		Button^ btnReset;
		PictureBox^ picOriginal;
		PictureBox^ picProcessed;
		Label^ lblOrigTitle;
		Label^ lblProcTitle;

		// أزرار القائمة الجانبية
		Button^ btnGray;
		Button^ btnNeg;
		Button^ btnLog;
		Button^ btnBitPlane;
		Button^ btnGraySlice;
		Button^ btnHistEq;
		Button^ btnShowHist;
		Button^ btnFlipX;
		Button^ btnFlipY;
		Button^ btnFlipBoth;
		Button^ btnZoom;
		Button^ btnSkew;

		// أزرار الفلاتر المكانية (Spatial Filters)
		Button^ btnTradFilter;
		Button^ btnPyramidalFilter;
		Button^ btnCircularFilter;
		Button^ btnConeFilter;

		// السلايدرز التفاعلية
		TrackBar^ trkRotate;
		Label^ lblRotateVal;
		TrackBar^ trkGamma;
		Label^ lblGammaVal;
		TrackBar^ trkTransX;
		Label^ lblTransXVal;
		TrackBar^ trkTransY;
		Label^ lblTransYVal;

		System::ComponentModel::Container^ components;

		// تحويل cv::Mat إلى Bitmap
		Bitmap^ MatToBitmap(const cv::Mat& mat)
		{
			if (mat.empty()) return nullptr;

			cv::Mat temp;
			if (mat.channels() == 1)
				cv::cvtColor(mat, temp, cv::COLOR_GRAY2BGR);
			else
				temp = mat.clone();

			Bitmap^ bmp = gcnew Bitmap(temp.cols, temp.rows, PixelFormat::Format24bppRgb);
			BitmapData^ bmpData = bmp->LockBits(
				System::Drawing::Rectangle(0, 0, bmp->Width, bmp->Height),
				ImageLockMode::WriteOnly, bmp->PixelFormat);

			for (int y = 0; y < temp.rows; y++) {
				memcpy((unsigned char*)bmpData->Scan0.ToPointer() + y * bmpData->Stride,
					temp.ptr(y), temp.cols * 3);
			}
			bmp->UnlockBits(bmpData);
			return bmp;
		}

		// تحويل System::Drawing::Bitmap إلى cv::Mat (آمن 100% مع كل المسارات)
		cv::Mat BitmapToMat(Bitmap^ bmp)
		{
			Bitmap^ bmp24 = gcnew Bitmap(bmp->Width, bmp->Height, PixelFormat::Format24bppRgb);
			Graphics^ g = Graphics::FromImage(bmp24);
			g->DrawImage(bmp, 0, 0, bmp->Width, bmp->Height);
			delete g;

			cv::Mat mat(bmp24->Height, bmp24->Width, CV_8UC3);
			BitmapData^ bmpData = bmp24->LockBits(
				System::Drawing::Rectangle(0, 0, bmp24->Width, bmp24->Height),
				ImageLockMode::ReadOnly, bmp24->PixelFormat);

			for (int y = 0; y < mat.rows; y++) {
				memcpy(mat.ptr(y),
					(unsigned char*)bmpData->Scan0.ToPointer() + y * bmpData->Stride,
					mat.cols * 3);
			}
			bmp24->UnlockBits(bmpData);
			delete bmp24;
			return mat;
		}

		void UpdateProcessedView()
		{
			if (!global_dst.empty()) {
				picProcessed->Image = MatToBitmap(global_dst);
			}
		}

		// تأثيرات مرور الماوس (Hover Effects)
		void OnBtnMouseEnter(Object^ sender, EventArgs^ e)
		{
			Button^ btn = safe_cast<Button^>(sender);
			btn->Tag = btn->BackColor;
			btn->BackColor = Color::FromArgb(56, 189, 248); // Sky Blue Glow
			btn->ForeColor = Color::FromArgb(15, 23, 42);
		}

		void OnBtnMouseLeave(Object^ sender, EventArgs^ e)
		{
			Button^ btn = safe_cast<Button^>(sender);
			if (btn->Tag != nullptr) {
				btn->BackColor = safe_cast<Color>(btn->Tag);
				btn->ForeColor = Color::White;
			}
		}

		Button^ CreateStyledButton(String^ text, int yPos, Color bgColor)
		{
			Button^ btn = gcnew Button();
			btn->Text = text;
			btn->Location = Point(15, yPos);
			btn->Size = System::Drawing::Size(205, 35);
			btn->FlatStyle = FlatStyle::Flat;
			btn->FlatAppearance->BorderSize = 0;
			btn->BackColor = bgColor;
			btn->ForeColor = Color::White;
			btn->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.5f, FontStyle::Bold);
			btn->Cursor = Cursors::Hand;
			btn->MouseEnter += gcnew EventHandler(this, &MyForm::OnBtnMouseEnter);
			btn->MouseLeave += gcnew EventHandler(this, &MyForm::OnBtnMouseLeave);
			return btn;
		}

		Label^ CreateSectionLabel(String^ text, int yPos)
		{
			Label^ lbl = gcnew Label();
			lbl->Text = text;
			lbl->Location = Point(15, yPos);
			lbl->AutoSize = true;
			lbl->ForeColor = Color::FromArgb(148, 163, 184);
			lbl->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f, FontStyle::Bold);
			return lbl;
		}

		void ApplyModernDarkTheme()
		{
			this->Text = L"VisionLab Pro | Image Processing Studio";
			this->Size = System::Drawing::Size(1320, 860);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->BackColor = Color::FromArgb(15, 23, 42);

			// 1. الشريط العلوي
			topPanel = gcnew Panel();
			topPanel->Dock = DockStyle::Top;
			topPanel->Height = 65;
			topPanel->BackColor = Color::FromArgb(30, 41, 59);
			this->Controls->Add(topPanel);

			lblTitle = gcnew Label();
			lblTitle->Text = L"VISIONLAB PRO  |  IMAGE STUDIO";
			lblTitle->Location = Point(25, 18);
			lblTitle->AutoSize = true;
			lblTitle->ForeColor = Color::FromArgb(56, 189, 248);
			lblTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 15, FontStyle::Bold);
			topPanel->Controls->Add(lblTitle);

			btnLoad = CreateStyledButton(L"+  Load Image", 14, Color::FromArgb(37, 99, 235));
			btnLoad->Location = Point(860, 14);
			btnLoad->Size = System::Drawing::Size(135, 38);
			btnLoad->Click += gcnew EventHandler(this, &MyForm::OnLoadClick);
			topPanel->Controls->Add(btnLoad);

			btnReset = CreateStyledButton(L"↺  Reset All", 14, Color::FromArgb(71, 85, 105));
			btnReset->Location = Point(1005, 14);
			btnReset->Size = System::Drawing::Size(120, 38);
			btnReset->Click += gcnew EventHandler(this, &MyForm::OnResetClick);
			topPanel->Controls->Add(btnReset);

			btnSave = CreateStyledButton(L"↓  Save Result", 14, Color::FromArgb(16, 185, 129));
			btnSave->Location = Point(1135, 14);
			btnSave->Size = System::Drawing::Size(140, 38);
			btnSave->Click += gcnew EventHandler(this, &MyForm::OnSaveClick);
			topPanel->Controls->Add(btnSave);

			// 2. القائمة الجانبية للأدوات
			sidePanel = gcnew Panel();
			sidePanel->Dock = DockStyle::Left;
			sidePanel->Width = 250;
			sidePanel->AutoScroll = true;
			sidePanel->BackColor = Color::FromArgb(30, 41, 59);
			this->Controls->Add(sidePanel);

			Color btnDark = Color::FromArgb(51, 65, 85);
			Color btnFilterCol = Color::FromArgb(67, 56, 202); // Indigo accent للفلاتر
			int y = 15;

			sidePanel->Controls->Add(CreateSectionLabel(L"INTENSITY & SLICING", y)); y += 25;
			btnGray = CreateStyledButton(L"Grayscale Convert", y, btnDark); btnGray->Click += gcnew EventHandler(this, &MyForm::OnGrayClick); sidePanel->Controls->Add(btnGray); y += 40;
			btnNeg = CreateStyledButton(L"Image Negative", y, btnDark); btnNeg->Click += gcnew EventHandler(this, &MyForm::OnNegClick); sidePanel->Controls->Add(btnNeg); y += 40;
			btnLog = CreateStyledButton(L"Log Transformation", y, btnDark); btnLog->Click += gcnew EventHandler(this, &MyForm::OnLogClick); sidePanel->Controls->Add(btnLog); y += 40;
			btnBitPlane = CreateStyledButton(L"Bit-Plane Slicing (64)", y, btnDark); btnBitPlane->Click += gcnew EventHandler(this, &MyForm::OnBitPlaneClick); sidePanel->Controls->Add(btnBitPlane); y += 40;
			btnGraySlice = CreateStyledButton(L"Gray-Level Slicing", y, btnDark); btnGraySlice->Click += gcnew EventHandler(this, &MyForm::OnGraySliceClick); sidePanel->Controls->Add(btnGraySlice); y += 48;

			sidePanel->Controls->Add(CreateSectionLabel(L"HISTOGRAM ANALYSIS", y)); y += 25;
			btnHistEq = CreateStyledButton(L"Histogram Equalization", y, Color::FromArgb(14, 116, 144)); btnHistEq->Click += gcnew EventHandler(this, &MyForm::OnHistEqClick); sidePanel->Controls->Add(btnHistEq); y += 40;
			btnShowHist = CreateStyledButton(L"Compare Histograms", y, Color::FromArgb(14, 116, 144)); btnShowHist->Click += gcnew EventHandler(this, &MyForm::OnShowHistClick); sidePanel->Controls->Add(btnShowHist); y += 48;

			sidePanel->Controls->Add(CreateSectionLabel(L"SPATIAL SMOOTHING FILTERS", y)); y += 25;
			btnTradFilter = CreateStyledButton(L"Traditional Filter (3×3)", y, btnFilterCol); btnTradFilter->Click += gcnew EventHandler(this, &MyForm::OnTradFilterClick); sidePanel->Controls->Add(btnTradFilter); y += 40;
			btnPyramidalFilter = CreateStyledButton(L"Pyramidal Filter (5×5)", y, btnFilterCol); btnPyramidalFilter->Click += gcnew EventHandler(this, &MyForm::OnPyramidalFilterClick); sidePanel->Controls->Add(btnPyramidalFilter); y += 40;
			btnCircularFilter = CreateStyledButton(L"Circular Filter (5×5)", y, btnFilterCol); btnCircularFilter->Click += gcnew EventHandler(this, &MyForm::OnCircularFilterClick); sidePanel->Controls->Add(btnCircularFilter); y += 40;
			btnConeFilter = CreateStyledButton(L"Cone Filter (5×5)", y, btnFilterCol); btnConeFilter->Click += gcnew EventHandler(this, &MyForm::OnConeFilterClick); sidePanel->Controls->Add(btnConeFilter); y += 48;

			sidePanel->Controls->Add(CreateSectionLabel(L"GEOMETRIC TRANSFORMS", y)); y += 25;
			btnFlipX = CreateStyledButton(L"Flip Horizontal (X)", y, btnDark); btnFlipX->Click += gcnew EventHandler(this, &MyForm::OnFlipXClick); sidePanel->Controls->Add(btnFlipX); y += 40;
			btnFlipY = CreateStyledButton(L"Flip Vertical (Y)", y, btnDark); btnFlipY->Click += gcnew EventHandler(this, &MyForm::OnFlipYClick); sidePanel->Controls->Add(btnFlipY); y += 40;
			btnFlipBoth = CreateStyledButton(L"Flip Both Axes", y, btnDark); btnFlipBoth->Click += gcnew EventHandler(this, &MyForm::OnFlipBothClick); sidePanel->Controls->Add(btnFlipBoth); y += 40;
			btnZoom = CreateStyledButton(L"Zoom 2x (Top-Left)", y, btnDark); btnZoom->Click += gcnew EventHandler(this, &MyForm::OnZoomClick); sidePanel->Controls->Add(btnZoom); y += 40;
			btnSkew = CreateStyledButton(L"Skew / Shear", y, btnDark); btnSkew->Click += gcnew EventHandler(this, &MyForm::OnSkewClick); sidePanel->Controls->Add(btnSkew); y += 30;

			// 3. مساحة عرض الصور
			lblOrigTitle = gcnew Label();
			lblOrigTitle->Text = L"ORIGINAL IMAGE";
			lblOrigTitle->Location = Point(280, 80);
			lblOrigTitle->AutoSize = true;
			lblOrigTitle->ForeColor = Color::FromArgb(148, 163, 184);
			lblOrigTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
			this->Controls->Add(lblOrigTitle);

			picOriginal = gcnew PictureBox();
			picOriginal->Location = Point(280, 108);
			picOriginal->Size = System::Drawing::Size(485, 480);
			picOriginal->BackColor = Color::FromArgb(30, 41, 59);
			picOriginal->SizeMode = PictureBoxSizeMode::Zoom;
			picOriginal->BorderStyle = BorderStyle::FixedSingle;
			this->Controls->Add(picOriginal);

			lblProcTitle = gcnew Label();
			lblProcTitle->Text = L"PROCESSED RESULT (LIVE)";
			lblProcTitle->Location = Point(790, 80);
			lblProcTitle->AutoSize = true;
			lblProcTitle->ForeColor = Color::FromArgb(56, 189, 248);
			lblProcTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
			this->Controls->Add(lblProcTitle);

			picProcessed = gcnew PictureBox();
			picProcessed->Location = Point(790, 108);
			picProcessed->Size = System::Drawing::Size(485, 480);
			picProcessed->BackColor = Color::FromArgb(30, 41, 59);
			picProcessed->SizeMode = PictureBoxSizeMode::Zoom;
			picProcessed->BorderStyle = BorderStyle::FixedSingle;
			this->Controls->Add(picProcessed);

			// 4. لوحة السلايدرز التفاعلية (Live Sliders Panel)
			slidersPanel = gcnew Panel();
			slidersPanel->Location = Point(280, 605);
			slidersPanel->Size = System::Drawing::Size(995, 130);
			slidersPanel->BackColor = Color::FromArgb(30, 41, 59);
			this->Controls->Add(slidersPanel);

			// Slider 1: Rotation (0 - 360)
			lblRotateVal = gcnew Label();
			lblRotateVal->Text = L"Rotation Angle: 0°";
			lblRotateVal->Location = Point(20, 12);
			lblRotateVal->AutoSize = true;
			lblRotateVal->ForeColor = Color::FromArgb(56, 189, 248);
			lblRotateVal->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.5f, FontStyle::Bold);
			slidersPanel->Controls->Add(lblRotateVal);

			trkRotate = gcnew TrackBar();
			trkRotate->Location = Point(20, 35);
			trkRotate->Size = System::Drawing::Size(220, 45);
			trkRotate->Minimum = 0;
			trkRotate->Maximum = 360;
			trkRotate->Value = 0;
			trkRotate->TickFrequency = 45;
			trkRotate->Scroll += gcnew EventHandler(this, &MyForm::OnRotateScroll);
			slidersPanel->Controls->Add(trkRotate);

			// Slider 2: Gamma (1 to 30 -> 0.1 to 3.0)
			lblGammaVal = gcnew Label();
			lblGammaVal->Text = L"Gamma (γ): 1.0";
			lblGammaVal->Location = Point(265, 12);
			lblGammaVal->AutoSize = true;
			lblGammaVal->ForeColor = Color::FromArgb(56, 189, 248);
			lblGammaVal->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.5f, FontStyle::Bold);
			slidersPanel->Controls->Add(lblGammaVal);

			trkGamma = gcnew TrackBar();
			trkGamma->Location = Point(265, 35);
			trkGamma->Size = System::Drawing::Size(220, 45);
			trkGamma->Minimum = 1;
			trkGamma->Maximum = 30;
			trkGamma->Value = 10;
			trkGamma->TickFrequency = 5;
			trkGamma->Scroll += gcnew EventHandler(this, &MyForm::OnGammaScroll);
			slidersPanel->Controls->Add(trkGamma);

			// Slider 3: Translate X (-150 to +150)
			lblTransXVal = gcnew Label();
			lblTransXVal->Text = L"Translate X: 0 px";
			lblTransXVal->Location = Point(510, 12);
			lblTransXVal->AutoSize = true;
			lblTransXVal->ForeColor = Color::FromArgb(56, 189, 248);
			lblTransXVal->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.5f, FontStyle::Bold);
			slidersPanel->Controls->Add(lblTransXVal);

			trkTransX = gcnew TrackBar();
			trkTransX->Location = Point(510, 35);
			trkTransX->Size = System::Drawing::Size(220, 45);
			trkTransX->Minimum = -150;
			trkTransX->Maximum = 150;
			trkTransX->Value = 0;
			trkTransX->TickFrequency = 50;
			trkTransX->Scroll += gcnew EventHandler(this, &MyForm::OnTranslateScroll);
			slidersPanel->Controls->Add(trkTransX);

			// Slider 4: Translate Y (-150 to +150)
			lblTransYVal = gcnew Label();
			lblTransYVal->Text = L"Translate Y: 0 px";
			lblTransYVal->Location = Point(755, 12);
			lblTransYVal->AutoSize = true;
			lblTransYVal->ForeColor = Color::FromArgb(56, 189, 248);
			lblTransYVal->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.5f, FontStyle::Bold);
			slidersPanel->Controls->Add(lblTransYVal);

			trkTransY = gcnew TrackBar();
			trkTransY->Location = Point(755, 35);
			trkTransY->Size = System::Drawing::Size(220, 45);
			trkTransY->Minimum = -150;
			trkTransY->Maximum = 150;
			trkTransY->Value = 0;
			trkTransY->TickFrequency = 50;
			trkTransY->Scroll += gcnew EventHandler(this, &MyForm::OnTranslateScroll);
			slidersPanel->Controls->Add(trkTransY);

			// 5. شريط معلومات الصورة
			lblInfo = gcnew Label();
			lblInfo->Text = L"Image Properties: Load an image to inspect dimensions, channels, and pixel intensity range.";
			lblInfo->Location = Point(280, 750);
			lblInfo->Size = System::Drawing::Size(995, 42);
			lblInfo->BackColor = Color::FromArgb(30, 41, 59);
			lblInfo->ForeColor = Color::FromArgb(226, 232, 240);
			lblInfo->Font = gcnew System::Drawing::Font(L"Consolas", 10, FontStyle::Regular);
			lblInfo->TextAlign = ContentAlignment::MiddleLeft;
			lblInfo->Padding = System::Windows::Forms::Padding(15, 0, 0, 0);
			this->Controls->Add(lblInfo);
		}

		void InitializeComponent(void)
		{
			this->SuspendLayout();
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1320, 860);
			this->Name = L"MyForm";
			this->ResumeLayout(false);
		}

		// =========================================================
		// أحداث التشغيل والتفاعل
		// =========================================================
		void ResetSlidersUI()
		{
			trkRotate->Value = 0; lblRotateVal->Text = L"Rotation Angle: 0°";
			trkGamma->Value = 10; lblGammaVal->Text = L"Gamma (γ): 1.0";
			trkTransX->Value = 0; lblTransXVal->Text = L"Translate X: 0 px";
			trkTransY->Value = 0; lblTransYVal->Text = L"Translate Y: 0 px";
		}

		void OnLoadClick(Object^ sender, EventArgs^ e)
		{
			OpenFileDialog^ ofd = gcnew OpenFileDialog();
			ofd->Filter = L"Image Files|*.jpg;*.jpeg;*.png;*.bmp;*.tif";
			if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				Bitmap^ loadedBmp = gcnew Bitmap(ofd->FileName);
				global_src = BitmapToMat(loadedBmp);
				delete loadedBmp;

				if (global_src.empty()) return;

				global_dst = global_src.clone();
				ResetSlidersUI();
				picOriginal->Image = MatToBitmap(global_src);
				picProcessed->Image = MatToBitmap(global_dst);

				cv::Mat grayTemp;
				cv::cvtColor(global_src, grayTemp, cv::COLOR_BGR2GRAY);
				double minVal, maxVal;
				cv::minMaxLoc(grayTemp, &minVal, &maxVal);

				lblInfo->Text = String::Format(L"Rows: {0}  |  Cols: {1}  |  Channels: {2}  |  Min Intensity: {3}  |  Max Intensity: {4}",
					global_src.rows, global_src.cols, global_src.channels(), minVal, maxVal);
			}
		}

		void OnResetClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			global_dst = global_src.clone();
			ResetSlidersUI();
			UpdateProcessedView();
		}

		void OnSaveClick(Object^ sender, EventArgs^ e)
		{
			if (global_dst.empty()) return;
			SaveFileDialog^ sfd = gcnew SaveFileDialog();
			sfd->Filter = L"PNG Image|*.png|JPEG Image|*.jpg";
			if (sfd->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				Bitmap^ saveBmp = MatToBitmap(global_dst);
				saveBmp->Save(sfd->FileName);
				delete saveBmp;
				MessageBox::Show(L"Image saved successfully!", L"VisionLab Pro", MessageBoxButtons::OK, MessageBoxIcon::Information);
			}
		}

		cv::Mat GetGraySource()
		{
			cv::Mat gray;
			if (global_src.channels() == 3)
				cv::cvtColor(global_src, gray, cv::COLOR_BGR2GRAY);
			else
				gray = global_src.clone();
			return gray;
		}

		// --- أحداث السلايدرز التفاعلية ---
		void OnRotateScroll(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			int deg = trkRotate->Value;
			lblRotateVal->Text = String::Format(L"Rotation Angle: {0}°", deg);

			double angle = deg * 3.14159265358979323846 / 180.0;
			double alpha = cos(angle), beta = sin(angle);
			double cx = global_src.cols / 2.0, cy = global_src.rows / 2.0;
			double r_data[6] = { alpha, beta, (1.0 - alpha) * cx - beta * cy, -beta, alpha, beta * cx + (1.0 - alpha) * cy };
			cv::Mat R = cv::Mat(2, 3, CV_64F, r_data).clone();
			cv::warpAffine(global_src, global_dst, R, global_src.size());
			UpdateProcessedView();
		}

		void OnGammaScroll(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			float gamma = trkGamma->Value / 10.0f;
			lblGammaVal->Text = String::Format(L"Gamma (γ): {0:F1}", gamma);

			cv::Mat gray = GetGraySource();
			cv::Mat gammaMat(gray.rows, gray.cols, CV_32FC1);
			for (int i = 0; i < gray.rows; i++)
				for (int j = 0; j < gray.cols; j++)
					gammaMat.at<float>(i, j) = powf(gray.at<uchar>(i, j), gamma);
			cv::normalize(gammaMat, gammaMat, 0, 255, cv::NORM_MINMAX);
			cv::convertScaleAbs(gammaMat, global_dst);
			UpdateProcessedView();
		}

		void OnTranslateScroll(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			int tx = trkTransX->Value;
			int ty = trkTransY->Value;
			lblTransXVal->Text = String::Format(L"Translate X: {0} px", tx);
			lblTransYVal->Text = String::Format(L"Translate Y: {0} px", ty);

			double tm_data[6] = { 1.0, 0.0, (double)tx, 0.0, 1.0, (double)ty };
			cv::Mat TM = cv::Mat(2, 3, CV_64F, tm_data).clone();
			cv::warpAffine(global_src, global_dst, TM, global_src.size());
			UpdateProcessedView();
		}

		// --- أحداث Intensity & Histogram ---
		void OnGrayClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			global_dst = GetGraySource();
			UpdateProcessedView();
		}

		void OnNegClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			global_dst = gray.clone();
			for (int i = 0; i < global_dst.rows; i++)
				for (int j = 0; j < global_dst.cols; j++)
					global_dst.at<uchar>(i, j) = 255 - gray.at<uchar>(i, j);
			UpdateProcessedView();
		}

		void OnLogClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			cv::Mat logMat(gray.rows, gray.cols, CV_32FC1);
			for (int i = 0; i < gray.rows; i++)
				for (int j = 0; j < gray.cols; j++)
					logMat.at<float>(i, j) = log(gray.at<uchar>(i, j) + 1.0f);
			cv::normalize(logMat, logMat, 0, 255, cv::NORM_MINMAX);
			cv::convertScaleAbs(logMat, global_dst);
			UpdateProcessedView();
		}

		void OnBitPlaneClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			global_dst = gray.clone();
			for (int i = 0; i < global_dst.rows; i++)
				for (int j = 0; j < global_dst.cols; j++)
					global_dst.at<uchar>(i, j) = (gray.at<uchar>(i, j) & 64) ? 255 : 0;
			UpdateProcessedView();
		}

		void OnGraySliceClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			global_dst = gray.clone();
			for (int i = 0; i < global_dst.rows; i++) {
				for (int j = 0; j < global_dst.cols; j++) {
					uchar p = gray.at<uchar>(i, j);
					global_dst.at<uchar>(i, j) = (p > 130 && p < 200) ? 255 : p;
				}
			}
			UpdateProcessedView();
		}

		void OnHistEqClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			cv::equalizeHist(gray, global_dst);
			UpdateProcessedView();
		}

		cv::Mat DrawHistMat(const cv::Mat& img, cv::Scalar color)
		{
			float hist[256] = { 0 };
			for (int r = 0; r < img.rows; r++)
				for (int c = 0; c < img.cols; c++)
					hist[img.at<uchar>(r, c)]++;

			int h = 250, w = 512;
			cv::Mat canvas(h, w, CV_8UC3, cv::Scalar(30, 41, 59));
			float maxVal = *std::max_element(hist, hist + 256);
			if (maxVal == 0) return canvas;

			for (int s = 0; s < 256; s++) {
				int binH = cvRound((hist[s] / maxVal) * (h - 20));
				cv::line(canvas, cv::Point(s * 2, h - 1), cv::Point(s * 2, h - 1 - binH), color, 2);
			}
			return canvas;
		}

		void OnShowHistClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat grayOrig = GetGraySource();
			cv::Mat grayProc = (global_dst.channels() == 3) ? GetGraySource() : global_dst;

			cv::Mat h1 = DrawHistMat(grayOrig, cv::Scalar(248, 189, 56));
			cv::Mat h2 = DrawHistMat(grayProc, cv::Scalar(129, 185, 16));

			cv::imshow("1. Original Histogram", h1);
			cv::imshow("2. Processed Histogram", h2);
		}

		// --- أحداث Spatial Smoothing Filters ---
		void OnTradFilterClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			float t_data[9] = {
				1, 1, 1,
				1, 1, 1,
				1, 1, 1
			};
			cv::Mat kernel_T = cv::Mat(3, 3, CV_32F, t_data).clone() / 9.0f;
			cv::filter2D(gray, global_dst, CV_8UC1, kernel_T);
			lblInfo->Text = L"Filter Applied: Traditional Filter (3x3)  |  Kernel Sum Divisor = 9";
			UpdateProcessedView();
		}

		void OnPyramidalFilterClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			float p_data[25] = {
				1, 2, 3, 2, 1,
				2, 4, 6, 4, 2,
				3, 6, 9, 6, 3,
				2, 4, 6, 4, 2,
				1, 2, 3, 2, 1
			};
			cv::Mat kernel_p = cv::Mat(5, 5, CV_32F, p_data).clone() / 81.0f;
			cv::filter2D(gray, global_dst, CV_8UC1, kernel_p);
			lblInfo->Text = L"Filter Applied: Pyramidal Filter (5x5)  |  Kernel Sum Divisor = 81";
			UpdateProcessedView();
		}

		void OnCircularFilterClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			float c_data[25] = {
				0, 1, 1, 1, 0,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 1, 1, 1, 0
			};
			cv::Mat kernel_c = cv::Mat(5, 5, CV_32F, c_data).clone() / 21.0f;
			cv::filter2D(gray, global_dst, CV_8UC1, kernel_c);
			lblInfo->Text = L"Filter Applied: Circular Filter (5x5)  |  Kernel Sum Divisor = 21";
			UpdateProcessedView();
		}

		void OnConeFilterClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::Mat gray = GetGraySource();
			float co_data[25] = {
				0, 0, 1, 0, 0,
				0, 2, 2, 2, 0,
				1, 2, 5, 2, 1,
				0, 2, 2, 2, 0,
				0, 0, 1, 0, 0
			};
			cv::Mat kernel_co = cv::Mat(5, 5, CV_32F, co_data).clone() / 25.0f;
			cv::filter2D(gray, global_dst, CV_8UC1, kernel_co);
			lblInfo->Text = L"Filter Applied: Cone Filter (5x5)  |  Kernel Sum Divisor = 25";
			UpdateProcessedView();
		}

		// --- أحداث Geometric Transforms ---
		void OnFlipXClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::flip(global_dst, global_dst, 0);
			UpdateProcessedView();
		}

		void OnFlipYClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::flip(global_dst, global_dst, 1);
			UpdateProcessedView();
		}

		void OnFlipBothClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty()) return;
			cv::flip(global_dst, global_dst, -1);
			UpdateProcessedView();
		}

		void OnZoomClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty() || global_dst.cols < 2 || global_dst.rows < 2) return;
			cv::Mat zoom = global_dst(cv::Rect(0, 0, global_dst.cols / 2, global_dst.rows / 2));
			cv::resize(zoom, global_dst, cv::Size(), 2.0, 2.0, cv::INTER_LINEAR);
			UpdateProcessedView();
		}

		void OnSkewClick(Object^ sender, EventArgs^ e)
		{
			if (global_src.empty() || global_dst.rows <= 1) return;
			double shear_val = -50.0 / (global_dst.rows - 1.0);
			double s_data[6] = { 1.0, shear_val, 0.0, 0.0, 1.0, 0.0 };
			cv::Mat S = cv::Mat(2, 3, CV_64F, s_data).clone();
			cv::warpAffine(global_dst, global_dst, S, global_dst.size());
			UpdateProcessedView();
		}
	};
}