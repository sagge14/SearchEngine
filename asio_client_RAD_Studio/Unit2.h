//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Samples.Gauges.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <boost_1_70/boost/asio.hpp>
#include <cstring>
#include <iostream>
#include <map>


using boost::asio::ip::tcp;


//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
	TButton *Button1;
	TRichEdit *RichEdit1;
	TListBox *ListBox1;
	TGauge *Gauge1;
	TLabel *Label1;
	TEdit *Edit2;
	TEdit *Edit3;
	TButton *Button2;
	TButton *Button3;
	TFileOpenDialog *FileOpenDialog1;
	TFileSaveDialog *FileSaveDialog1;
	TButton *Button4;
	TComboBox *ComboBox1;
	TTimer *Timer1;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ListBox1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall ComboBox1KeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
	enum { max_length = 1024 };
	enum class COMMAND : uint_fast64_t
	{
		SOLOREQUEST = 1,
		FILETEXT,
		JSONREGUEST,
        SOMEERROR
	};
	struct header
	{
		uint_fast64_t size{0};
		COMMAND command{0};
	};
	void __fastcall componentsEnable(bool b);

public:		// User declarations

	boost::asio::io_context io_context;
	tcp::resolver *resolver;
	tcp::socket *socket;
	std::map<String,double> myMap;
	header myHeader;
	void socketWrite(const String& s, COMMAND command);
	bool socketRead(std::string& s);
	std::string requests;
	void clear();
	std::map<String,std::map<String,double>> mapAnswers;

	__fastcall TForm2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
