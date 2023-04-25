object Form2: TForm2
  AlignWithMargins = True
  Left = 839
  Top = 393
  AutoSize = True
  Caption = 'SearchClient_asio'
  ClientHeight = 616
  ClientWidth = 785
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDesigned
  Scaled = False
  OnClose = FormClose
  TextHeight = 15
  object Gauge1: TGauge
    AlignWithMargins = True
    Left = 8
    Top = 545
    Width = 582
    Height = 34
    Enabled = False
    Progress = 0
  end
  object Label1: TLabel
    AlignWithMargins = True
    Left = 8
    Top = 39
    Width = 124
    Height = 28
    Caption = 'Relative index:'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -20
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
  end
  object Button1: TButton
    AlignWithMargins = True
    Left = 596
    Top = 585
    Width = 186
    Height = 28
    Caption = 'Enter'
    Enabled = False
    TabOrder = 0
    OnClick = Button1Click
  end
  object RichEdit1: TRichEdit
    AlignWithMargins = True
    Left = 8
    Top = 73
    Width = 582
    Height = 466
    Enabled = False
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object ListBox1: TListBox
    AlignWithMargins = True
    Left = 596
    Top = 73
    Width = 186
    Height = 506
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Segoe UI'
    Font.Style = []
    ItemHeight = 17
    ParentFont = False
    TabOrder = 2
    OnClick = ListBox1Click
  end
  object Edit2: TEdit
    AlignWithMargins = True
    Left = 4
    Top = 4
    Width = 117
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    Text = '37.79.202.171'
    TextHint = 'Ip (37.79.202.171)'
  end
  object Edit3: TEdit
    Left = 129
    Top = 4
    Width = 117
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    Text = '15001'
    TextHint = 'Port (15001)'
  end
  object Button2: TButton
    Left = 254
    Top = 6
    Width = 72
    Height = 24
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Connect'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 596
    Top = 10
    Width = 180
    Height = 24
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Send requests (json)'
    Enabled = False
    TabOrder = 6
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 597
    Top = 42
    Width = 179
    Height = 24
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Show answers (json)'
    Enabled = False
    TabOrder = 7
    OnClick = Button4Click
  end
  object ComboBox1: TComboBox
    Left = 9
    Top = 587
    Width = 580
    Height = 23
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Enabled = False
    TabOrder = 8
    TextHint = 'Enter your request'
    OnChange = ComboBox1Change
    OnKeyUp = ComboBox1KeyUp
  end
  object FileOpenDialog1: TFileOpenDialog
    FavoriteLinks = <>
    FileTypes = <>
    Options = []
    Left = 44
    Top = 80
  end
  object FileSaveDialog1: TFileSaveDialog
    FavoriteLinks = <>
    FileName = 'C:\Users\user\Documents\Embarcadero\Studio\Projects'
    FileTypes = <>
    Options = []
    Title = #1042#1074#1077#1076#1080#1090#1077' '#1080#1084#1103' '#1092#1072#1081#1083#1072' '#1076#1083#1103' '#1089#1086#1093#1088#1072#1085#1077#1085#1080#1103' '#1088#1077#1079#1091#1083#1100#1090#1072#1090#1086#1074' '#1079#1072#1087#1088#1086#1089#1072
    Left = 124
    Top = 88
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 40
    OnTimer = Timer1Timer
    Left = 320
    Top = 224
  end
end
