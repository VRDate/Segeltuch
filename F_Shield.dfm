object ShieldForm: TShieldForm
  Left = 709
  Top = 190
  ActiveControl = btnOK
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Shield Tool'
  ClientHeight = 381
  ClientWidth = 401
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlGenerate: TPanel
    Left = 8
    Top = 80
    Width = 385
    Height = 265
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 2
    object Label1: TLabel
      Left = 224
      Top = 121
      Width = 153
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Top-View'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label2: TLabel
      Left = 224
      Top = 248
      Width = 153
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Side-View'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label3: TLabel
      Left = 16
      Top = 16
      Width = 155
      Height = 13
      Caption = 'Dimensions of Entire Model'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label4: TLabel
      Left = 16
      Top = 88
      Width = 64
      Height = 13
      Caption = 'Shield Size'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label5: TLabel
      Left = 16
      Top = 136
      Width = 77
      Height = 13
      Caption = 'Shield Center'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label6: TLabel
      Left = 16
      Top = 232
      Width = 71
      Height = 13
      Caption = 'Preview Zoom:'
    end
    object Label7: TLabel
      Left = 16
      Top = 184
      Width = 114
      Height = 13
      Caption = 'Number of Divisions'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object pnlTop: TPanel
      Left = 224
      Top = 12
      Width = 153
      Height = 110
      BevelOuter = bvNone
      BorderStyle = bsSingle
      Color = clBlack
      TabOrder = 0
      object boxTop: TPaintBox
        Left = 0
        Top = 0
        Width = 149
        Height = 106
        Align = alClient
        OnPaint = boxTopPaint
      end
    end
    object pnlSide: TPanel
      Left = 224
      Top = 139
      Width = 153
      Height = 110
      BevelOuter = bvNone
      BorderStyle = bsSingle
      Color = clBlack
      TabOrder = 1
      object boxSide: TPaintBox
        Left = 0
        Top = 0
        Width = 149
        Height = 106
        Align = alClient
        OnPaint = boxSidePaint
      end
    end
    object edtSize: TEdit
      Left = 16
      Top = 104
      Width = 201
      Height = 21
      TabOrder = 2
      OnChange = edtSizeChange
    end
    object edtCenter: TEdit
      Left = 16
      Top = 152
      Width = 201
      Height = 21
      TabOrder = 3
      OnChange = edtCenterChange
    end
    object mmoDim: TMemo
      Left = 16
      Top = 32
      Width = 201
      Height = 41
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 4
    end
    object tbrZoom: TTrackBar
      Left = 88
      Top = 232
      Width = 134
      Height = 25
      Max = 1000
      Orientation = trHorizontal
      Frequency = 1
      Position = 600
      SelEnd = 0
      SelStart = 0
      TabOrder = 5
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbrZoomChange
    end
    object edtNumDiv: TEdit
      Left = 16
      Top = 200
      Width = 185
      Height = 21
      TabOrder = 6
      Text = '6'
    end
    object UpDown1: TUpDown
      Left = 201
      Top = 200
      Width = 15
      Height = 21
      Associate = edtNumDiv
      Min = 4
      Position = 6
      TabOrder = 7
      Wrap = False
    end
  end
  object rdoGen: TRadioButton
    Left = 24
    Top = 72
    Width = 161
    Height = 17
    Caption = 'Generate Shield as an Ellipse'
    Checked = True
    TabOrder = 1
    TabStop = True
  end
  object pnlImport: TPanel
    Left = 8
    Top = 16
    Width = 385
    Height = 49
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
    object lblDXFName: TLabel
      Left = 96
      Top = 16
      Width = 3
      Height = 13
    end
    object Button1: TButton
      Left = 16
      Top = 16
      Width = 75
      Height = 23
      Caption = 'Load DXF'
      TabOrder = 0
      OnClick = Button1Click
    end
  end
  object rdoImport: TRadioButton
    Left = 24
    Top = 8
    Width = 161
    Height = 17
    Caption = 'Import Shield Data from DXF'
    TabOrder = 0
  end
  object btnOK: TButton
    Left = 240
    Top = 352
    Width = 75
    Height = 23
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 4
  end
  object btnCancel: TButton
    Left = 320
    Top = 352
    Width = 75
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 5
  end
  object dlgOpen: TOpenDialog
    Filter = 'AutoCAD R12 DXF Files (*.dxf)|*.dxf'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Title = 'Select DXF File to use as shields'
    Left = 248
    Top = 32
  end
end
