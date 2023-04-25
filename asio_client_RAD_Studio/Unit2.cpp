//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <System.JSON.hpp>
#include "stdint.h"
#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "frxClass"
#pragma link "frxClass"
#pragma link "frxClass"
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner), myHeader{}
{}
//---------------------------------------------------------------------------
void TForm2::socketWrite(const String& str, COMMAND command)
{
    myHeader.size = str.Length();
	myHeader.command = command;

	try
	{
		boost::asio::write(*socket, boost::asio::buffer(&myHeader, sizeof(myHeader)));
		boost::asio::write(*socket, boost::asio::buffer(AnsiString(str).c_str(), myHeader.size));
	}
	catch (std::exception& e)
	{
		componentsEnable(false);
		ShowMessage(e.what());
	}

}
bool TForm2::socketRead(std::string& str)
{

	try
	{
		socket->read_some(boost::asio::buffer(&myHeader, sizeof (myHeader)));
		if(myHeader.command	== COMMAND::SOMEERROR)
		{
			MessageDlg("Server error!", mtError, TMsgDlgButtons() << mbOK,0);
			componentsEnable(false);
			ComboBox1->Text = "";
			return false;
		}


		size_t count = 0;
		Gauge1->MaxValue = myHeader.size;

		while(count < myHeader.size)
		{
			char reply[max_length];
			size_t reply_length = socket->read_some(boost::asio::buffer(reply, max_length));
			str += std::string(reply,reply_length);
			count += reply_length;
			Gauge1->AddProgress(max_length);
		}
	}
    catch (std::exception& e)
	{
		componentsEnable(false);
		ShowMessage(e.what());
		return false;
	}

	return true;
}
void __fastcall TForm2::Button1Click(TObject *Sender)
{
	if(ComboBox1->Text == "")
		return;


	clear();
	ListBox1->Clear();
	ComboBox1->Items->Clear();

	socketWrite(ComboBox1->Text, COMMAND::SOLOREQUEST);
	std::string str;

	if(!socketRead(str))
	return;

	auto sl = std::make_unique<TStringList>();
	sl->Text = str.c_str();

	myMap.clear();

	for(const auto& i:sl.get())
	{
		auto buf = std::make_unique<TStringList>();
		buf.get()->DelimitedText = i;
		myMap.emplace(buf->Strings[0], std::atof(AnsiString(buf.get()->Strings[1]).c_str())) ;
		ListBox1->Items->Add(buf.get()->Strings[0]);
	}

}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormClose(TObject *Sender, TCloseAction &Action)
{
	delete socket;
	delete resolver;
}
void TForm2::clear()
{
    RichEdit1->Text = "";
	Gauge1->Progress = 0;
	Label1->Caption = "Relative index:";
}
//---------------------------------------------------------------------------
void __fastcall TForm2::ListBox1Click(TObject *Sender)
{
	clear();
	String rel = myMap.at(ListBox1->Items->Strings[ListBox1->ItemIndex]);
	Label1->Caption +=  rel;

	socketWrite(ListBox1->Items->Strings[ListBox1->ItemIndex], COMMAND::FILETEXT);

	std::string str;

	if(!socketRead(str))
		return;

	RichEdit1->Text = str.c_str();
}
//---------------------------------------------------------------------------

void __fastcall TForm2::Button2Click(TObject *Sender)
{
	if(socket != nullptr || resolver != nullptr)
	{
		delete socket;
		delete resolver;
	}

	try
	{
		socket = new tcp::socket(io_context);
		resolver = new tcp::resolver(io_context);
		boost::asio::connect(*socket, resolver->resolve(AnsiString(Edit2->Text).c_str()	, AnsiString(Edit3->Text).c_str()));

		componentsEnable(true);

		ShowMessage("The connection to the server was successful!");

		if(Edit2->Text == "")
			Edit2->Text = "localhost";
	}
	catch (std::exception& e)
	{
		ShowMessage(e.what());
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm2::Button3Click(TObject *Sender)
{
	FileOpenDialog1->FileName = "Requests.json";
	FileOpenDialog1->Title = "Select the request file";
	FileOpenDialog1->FileTypes->Add();
	FileOpenDialog1->DefaultFolder = GetCurrentDir();
	FileOpenDialog1->FileTypes->operator[](0)->FileMask = "*.json";
	FileOpenDialog1->FileTypes->operator[](0)->DisplayName = "Files json";

	if(FileOpenDialog1->Execute())
	{
		auto buf = std::make_unique<TStringList>();
		buf->LoadFromFile(FileOpenDialog1->FileName);
		String ans = buf->Text;

		socketWrite(ans, COMMAND::JSONREGUEST);

		std::string str;

		if(!socketRead(str))
			return;

		String jsonn = str.c_str();
		jsonn = ((TJSONObject*)TJSONObject::ParseJSONValue(jsonn))->Format(2);

		FileSaveDialog1->DefaultFolder = GetCurrentDir();
		FileSaveDialog1->FileTypes->Add();
		FileSaveDialog1->FileTypes = FileOpenDialog1->FileTypes;
		FileSaveDialog1->DefaultFolder = GetCurrentDir();
		FileSaveDialog1->FileName = "Answers.json";

		if(FileSaveDialog1->Execute())
		{
			auto buf2 = std::make_unique<TStringStream>();
			buf2->WriteString(jsonn);
			buf2->SaveToFile(FileSaveDialog1->FileName);
		}

	}
}
//---------------------------------------------------------------------------


void __fastcall TForm2::Button4Click(TObject *Sender)
{
	clear();

	ComboBox1->Text = "";
	ListBox1->Clear()	;
	FileOpenDialog1->FileName = "Answers.json";
	FileOpenDialog1->Title = "Select the answer file";
	FileOpenDialog1->FileTypes->Add();
	FileOpenDialog1->DefaultFolder = GetCurrentDir();
	FileOpenDialog1->FileTypes->operator[](0)->FileMask = "*.json";
	FileOpenDialog1->FileTypes->operator[](0)->DisplayName = "Files json";

	try
	{
	if(FileOpenDialog1->Execute())
	{
		auto buf = std::make_unique<TStringStream>();
		buf->LoadFromFile(FileOpenDialog1->FileName);
		TJSONObject* jsonFile = (TJSONObject*)TJSONObject::ParseJSONValue(buf->DataString);
		TJSONArray* ar =  (TJSONArray*)TJSONObject::ParseJSONValue(jsonFile->GetValue("Answers")->ToString());
		int c = ar->Count ;
		for(int i = 0; i < ar->Count; i++)
		{
			TJSONObject* item = (TJSONObject*)TJSONObject::ParseJSONValue("{"+ar->Items[i]->ToString()+"}");
			TJSONPair* pairReqRes = item->Get(0);
			String req = pairReqRes->JsonString->Value();
			String res = pairReqRes->JsonValue->ToString()	;
			TJSONObject* resJ = (TJSONObject*)TJSONObject::ParseJSONValue(res);
			String result = resJ->GetValue("result")->ToString();

			if(result == "true")
			{
				String buf = resJ->GetValue("relevance")->ToString();
				TJSONArray* arRel =  (TJSONArray*)TJSONObject::ParseJSONValue(buf);
				for(int i = 0; i < arRel->Count; i++)
				{
					TJSONArray* arRelRes =  (TJSONArray*)TJSONObject::ParseJSONValue(arRel->Items[i]->ToString());
					String doc =  arRelRes->Items[0]->ToString();
					doc = StringReplace(doc, "\"", "", TReplaceFlags()<< rfReplaceAll << rfIgnoreCase);
					String resOne =  arRelRes->Items[1]->ToString();
					mapAnswers[req].insert(std::make_pair(doc,std::atof(AnsiString(resOne).c_str())));

					delete arRelRes;
				}
				delete arRel;
			}
			else
				mapAnswers[req];

			delete resJ;
			delete item;
		 }
		for(const auto&i:mapAnswers)
			ComboBox1->Items->Add(i.first);

		delete jsonFile;
		ComboBox1->DroppedDown = true;
	}
	}
	catch(...)
	{
		MessageDlg("JSON parse error!", mtError, TMsgDlgButtons() << mbOK	,0);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm2::ComboBox1Change(TObject *Sender)
{
	if(ComboBox1->Items->Count == 0)
		return;

	clear();
	ListBox1->Clear();
	String req = ComboBox1->Text;

    if(mapAnswers.find(req) != mapAnswers.end())
		myMap = mapAnswers.at(req);
	else
		return;

	std::vector<String> bufVec;
	for(const auto& i:mapAnswers.at(req))
		bufVec.push_back(i.first);

	std::sort(bufVec.begin(),bufVec.end(),[this, &req](const auto& i1,const auto& i2)
	{
		return mapAnswers.at(req).at(i2) < mapAnswers.at(req).at(i1);
	});

	for(const auto& i:bufVec)
		ListBox1->Items->Add(i);

}
//---------------------------------------------------------------------------

void __fastcall TForm2::ComboBox1KeyUp(TObject *Sender, WORD &Key, TShiftState Shift)

{
	if(Key == VK_RETURN)
		Button1->Click();

}
//---------------------------------------------------------------------------

void __fastcall TForm2::Timer1Timer(TObject *Sender)
{
	ComboBox1->Text = "1234";
	Button1->Click();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::componentsEnable(bool b)
{
		RichEdit1->Enabled = b;
		ListBox1->Enabled  = b;
		Gauge1->Enabled = b;
		Button1->Enabled = b;
		Button3->Enabled = b;
		Button4->Enabled = b;
		ComboBox1->Enabled = b;
		Label1->Enabled = b;
}

