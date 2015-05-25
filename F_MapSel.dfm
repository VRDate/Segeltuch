object MapSelForm: TMapSelForm
  Left = 271
  Top = 164
  Width = 508
  Height = 313
  Caption = 'Select Map File'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object StatusBar1: TStatusBar
    Left = 0
    Top = 267
    Width = 500
    Height = 19
    Panels = <>
    SimplePanel = False
  end
  object lvwFiles: TListView
    Left = 0
    Top = 0
    Width = 500
    Height = 237
    Align = alClient
    Columns = <>
    HideSelection = False
    HotTrackStyles = [htUnderlineHot]
    ReadOnly = True
    TabOrder = 1
    ViewStyle = vsList
    OnChange = lvwFilesChange
  end
  object Panel1: TPanel
    Left = 0
    Top = 237
    Width = 500
    Height = 30
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    object btnOK: TBitBtn
      Left = 344
      Top = 4
      Width = 75
      Height = 23
      Anchors = [akRight, akBottom]
      Caption = 'OK'
      Default = True
      ModalResult = 1
      TabOrder = 0
      OnClick = btnOKClick
    end
    object btnCancel: TBitBtn
      Left = 424
      Top = 4
      Width = 75
      Height = 23
      Anchors = [akRight, akBottom]
      Caption = 'Cancel'
      ModalResult = 2
      TabOrder = 1
    end
    object flbFiles: TFileListBox
      Left = 112
      Top = 8
      Width = 113
      Height = 17
      ExtendedSelect = False
      ItemHeight = 13
      Mask = '*.pcx'
      TabOrder = 2
      Visible = False
    end
  end
end
