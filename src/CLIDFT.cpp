#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
#using <OpenCVSharp.dll>
#using <OpenCVSharp.CPlusPlus.dll>
#using <OpenCVSharp.Extensions.dll>
using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace OpenCvSharp;
using namespace OpenCvSharp::CPlusPlus;
using namespace OpenCvSharp::Extensions;

ref class MyForm : public System::Windows::Forms::Form{
public:
	MyForm(){
		InitializeComponent();
	}
private:
	Mat ^img;
	Mat ^chkMask;
	VideoCapture ^cap;
	// UI
	PictureBox ^camPic;
	PictureBox ^FFTPic;
	PictureBox ^IFFTPic;
	PictureBox ^FilterPic;
	Button ^buttonClear;
	Timer ^timer1;
	Collections::Generic::List <Drawing::Point>^ pts;
	Graphics ^fltg;
	Bitmap ^fltbmp;
	System::ComponentModel::IContainer ^components;

	void InitializeComponent(void){
		//UIの構成
		components = gcnew System::ComponentModel::Container();
		camPic = gcnew PictureBox();
		FFTPic = gcnew PictureBox();
		IFFTPic = gcnew PictureBox();
		FilterPic = gcnew PictureBox();
		buttonClear = gcnew Button();

		timer1 = gcnew Timer(components);
		timer1->Enabled = true;
		timer1->Interval = 10;
		timer1->Tick += gcnew EventHandler(this, &MyForm::timer1_Tick);
		camPic->Location = Drawing::Point(14, 13);
		camPic->Size = Drawing::Size(480, 320);
		camPic->SizeMode = PictureBoxSizeMode::StretchImage;
		Controls->Add(camPic);
		FFTPic->Location = Drawing::Point(504, 13);
		FFTPic->Size = Drawing::Size(480, 320);
		FFTPic->SizeMode = PictureBoxSizeMode::StretchImage;
		Controls->Add(FFTPic);
		IFFTPic->Location = Drawing::Point(14, 341);
		IFFTPic->Size = Drawing::Size(480, 320);
		IFFTPic->SizeMode = PictureBoxSizeMode::StretchImage;
		Controls->Add(IFFTPic);
		FilterPic->Location = Drawing::Point(504, 341);
		FilterPic->Size = Drawing::Size(480, 320);
		FilterPic->SizeMode = PictureBoxSizeMode::StretchImage;
		FilterPic->Paint += gcnew PaintEventHandler(this, &MyForm::FilterPic_Paint);
		FilterPic->MouseMove += gcnew MouseEventHandler(this, &MyForm::FilterPic_MouseMove);
		Controls->Add(FilterPic);
		buttonClear->Location = Drawing::Point(0, 672);
		buttonClear->Size = Drawing::Size(1003, 56);
		buttonClear->Text = L"Clear";
		buttonClear->Click += gcnew EventHandler(this, &MyForm::buttonClear_Click);
		Controls->Add(buttonClear);

		ClientSize = Drawing::Size(1003, 728);
		Load += gcnew EventHandler(this, &MyForm::MyForm_Load);
		FormClosed += gcnew FormClosedEventHandler(this, &MyForm::MyForm_FormClosed);
		Text = "CLI Test PG";
	}
private: 
	void FilterPic_MouseMove(System::Object^  sender, MouseEventArgs^  e) {
		//フィルタマスクの描画と同時に点を記録
		//もたつくのでMouseDownに持って行った方が良いかも
		if (e->Button == Windows::Forms::MouseButtons::Left)
		{
			pts->Add(Drawing::Point(e->X, e->Y));
		}
		this->Refresh();
	}
	void FilterPic_Paint(System::Object^  sender, PaintEventArgs^  e) {
		//マスク画像のptsの座標に丸を書く
		if (pts->Count == 0){
			return;
		}
		SolidBrush ^wb = gcnew SolidBrush(Color::White);
		for each (Drawing::Point pt in pts)
		{
			fltg->FillEllipse(wb, pt.X - 4, pt.Y - 4, 9, 9);
		}
	}
	void buttonClear_Click(System::Object^  sender, System::EventArgs^  e) {
		//Reset
		SolidBrush ^b = gcnew SolidBrush(Color::Black);
		fltg->FillRectangle(b, 0, 0, fltbmp->Width, fltbmp->Height);
		pts->Clear();
		this->Refresh();
	}
	void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {
		cap = gcnew VideoCapture(0);
		Mat ^tmp = gcnew Mat();
		chkMask = nullptr;
		pts = gcnew Collections::Generic::List < Drawing::Point >;
		fltbmp = gcnew Bitmap(FilterPic->Width, FilterPic->Height);
		fltg = Graphics::FromImage(fltbmp);
		SolidBrush ^b = gcnew SolidBrush(Color::Black);
		fltg->FillRectangle(b, 0, 0, fltbmp->Width, fltbmp->Height);
		FilterPic->Image = fltbmp;
		this->Refresh();
	}
	void MyForm_FormClosed(System::Object^  sender, FormClosedEventArgs^  e) {
		cap->Release();
	}
	void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
		Mat ^frame = gcnew Mat();
		cap->Read(frame);
		if (!frame->Empty())
		{
			Cv2::Resize(frame, frame, CPlusPlus::Size(camPic->Width, camPic->Height), 0, 0, Interpolation::Linear);
			Image ^im = BitmapConverter::ToBitmap(frame);

			Mat ^gframe = gcnew Mat();
			Cv2::CvtColor(frame, gframe, ColorConversion::BgrToGray, 0);
			Mat ^padded = gcnew Mat();
			int m = Cv2::GetOptimalDFTSize(gframe->Rows);
			int n = Cv2::GetOptimalDFTSize(gframe->Cols);
			Cv2::CopyMakeBorder(gframe, padded, 0, m - gframe->Rows, 0, n - gframe->Cols, BorderType::Constant, Scalar::All(0));
			Mat ^padded32 = gcnew Mat();
			padded->ConvertTo(padded32, MatType::CV_32F, 1 / 255.0, 0);

			//fftshiftの代わりに原画像と(-1)^(x+y)の市松模様の乗算
			array <Mat^>^ planes = gcnew array<Mat^>(2);
			planes[0] = Mat::Zeros(padded->Size(), MatType::CV_32F);
			//chkMask for fftshift
			if (chkMask == nullptr){
				chkMask = gcnew Mat(padded->Size(), MatType::CV_32F);
				Mat::Indexer<float>^ negid = chkMask->GetGenericIndexer<float>();
				for (int y = 0; y < chkMask->Rows; y++){
					for (int x = 0; x < chkMask->Cols; x++){
						negid[y, x] = (float)Math::Pow(-1, x + y);
					}
				}
			}
			planes[0] = padded32->Mul(chkMask, 1.0);
			planes[1] = Mat::Zeros(padded->Size(), MatType::CV_32F);
			Mat ^complex = gcnew Mat();
			Cv2::Merge(planes, complex);
			Mat ^dft = gcnew Mat();
			Cv2::Dft(complex, dft, DftFlag2::None, 0);
			array <Mat^>^ dftplanes = gcnew array<Mat^>(2);
			Cv2::Split(dft, dftplanes);
			Mat ^magnitude = gcnew Mat();
			Cv2::Magnitude(dftplanes[0], dftplanes[1], magnitude);
			magnitude += Scalar::All(1);
			Cv2::Log(magnitude, magnitude);

			Mat ^invdft = gcnew Mat();
			Mat ^fltimg = BitmapConverter::ToMat(fltbmp);
			Mat ^gfltimg = gcnew Mat();
			Cv2::CvtColor(fltimg, gfltimg, ColorConversion::BgrToGray, 0);
			gfltimg->ConvertTo(gfltimg, MatType::CV_32FC1, 1, 0);

			dftplanes[0] = dftplanes[0]->Mul(gfltimg, 1 / 255.0);
			dftplanes[1] = dftplanes[1]->Mul(gfltimg, 1 / 255.0);
			Cv2::Merge(dftplanes, dft);
			Cv2::Dft(dft, invdft, DftFlag2::Inverse | DftFlag2::RealOutput, 0);
			invdft = invdft->Mul(chkMask, 1.0);
			Cv2::Normalize(invdft, invdft, 0, 1, NormType::MinMax, -1, nullptr);
			Mat ^invdft8 = gcnew Mat();
			Cv2::ConvertScaleAbs(invdft, invdft8, 255, 0);
			IFFTPic->Image = BitmapConverter::ToBitmap(invdft8);

			Cv2::Normalize(magnitude, magnitude, 0, 1, NormType::MinMax, -1, nullptr);
			Mat ^magnitude8 = gcnew Mat();
			Cv2::ConvertScaleAbs(magnitude, magnitude8, 255, 0);
			FFTPic->Image = BitmapConverter::ToBitmap(magnitude8);
			camPic->Image = im;
		}

	}
};

//[STAThreadAttribute]
void main(){
	Application::Run(gcnew MyForm());
}