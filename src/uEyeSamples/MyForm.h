#pragma once

namespace Project1 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();

			displayHandle = DisplayWindow->Handle;
			InitCamera();
		}

	protected:
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
			m_Camera->Exit();

		}

	private:
		System::ComponentModel::Container ^components;

	private: System::Windows::Forms::Button^  button_Save;

	private: System::Windows::Forms::PictureBox^  DisplayWindow;
		uEye::Camera ^m_Camera;
		Int32 m_s32MemID;
		IntPtr displayHandle;

		void InitCamera()
		{
			m_Camera = gcnew uEye::Camera();
			uEye::Defines::Status statusRet = uEye::Defines::Status::NO_SUCCESS;
			statusRet = m_Camera->Init();
			if (statusRet != uEye::Defines::Status::SUCCESS)
			{
				MessageBox::Show("Camera Initializing Failed");
			}
			m_Camera->Parameter->Load("pathToProfileFile/uEyeHDRProfile.ini");
			//Memory
			statusRet = m_Camera->Memory->Allocate(m_s32MemID, true);
			if (statusRet != uEye::Defines::Status::SUCCESS)
			{
				MessageBox::Show("Allocate Memory Failed");
			}
			statusRet = m_Camera->Acquisition->Capture();
			if (statusRet != uEye::Defines::Status::SUCCESS)
			{
				MessageBox::Show("Start Live Video Failed");
			}

			m_Camera->EventFrame += gcnew System::EventHandler(this, &MyForm::onFrameEvent);
			
		}

		void onFrameEvent(System::Object^  sender, System::EventArgs^  e)
		{
			m_Camera->Memory->GetActive(m_s32MemID);
			m_Camera->Display->Render(m_s32MemID, displayHandle);
		}


#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->button_Save = (gcnew System::Windows::Forms::Button());
			this->DisplayWindow = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->DisplayWindow))->BeginInit();
			this->SuspendLayout();
			// 
			// button_Save
			// 
			this->button_Save->Location = System::Drawing::Point(444, 63);
			this->button_Save->Name = L"button_Save";
			this->button_Save->Size = System::Drawing::Size(75, 23);
			this->button_Save->TabIndex = 0;
			this->button_Save->Text = L"Save";
			this->button_Save->UseVisualStyleBackColor = true;
			this->button_Save->Click += gcnew System::EventHandler(this, &MyForm::button_Save_Click);
			// 
			// DisplayWindow
			// 
			this->DisplayWindow->Location = System::Drawing::Point(12, 12);
			this->DisplayWindow->Name = L"DisplayWindow";
			this->DisplayWindow->Size = System::Drawing::Size(417, 314);
			this->DisplayWindow->TabIndex = 1;
			this->DisplayWindow->TabStop = false;
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(547, 338);
			this->Controls->Add(this->DisplayWindow);
			this->Controls->Add(this->button_Save);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->DisplayWindow))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void button_Save_Click(System::Object^  sender, System::EventArgs^  e) {
				 m_Camera->Memory->GetActive(m_s32MemID);
				 m_Camera->Image->Save("test.png",m_s32MemID, System::Drawing::Imaging::ImageFormat::Png,100);
			 }
};
}
