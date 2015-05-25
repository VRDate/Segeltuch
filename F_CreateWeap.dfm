object CreateWeapForm: TCreateWeapForm
  Left = 549
  Top = 250
  AutoScroll = False
  BorderIcons = [biMinimize, biMaximize]
  Caption = 'Create Weapons on Face'
  ClientHeight = 323
  ClientWidth = 575
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 333
    Top = 0
    Width = 3
    Height = 304
    Cursor = crHSplit
    Align = alRight
    Beveled = True
    ResizeStyle = rsUpdate
  end
  object stsMain: TStatusBar
    Left = 0
    Top = 304
    Width = 575
    Height = 19
    Panels = <>
    SimplePanel = False
    Visible = False
  end
  object Panel1: TPanel
    Left = 336
    Top = 0
    Width = 239
    Height = 304
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 1
    object Label1: TLabel
      Left = 8
      Top = 8
      Width = 101
      Height = 13
      Caption = 'Number of Weapons:'
    end
    object Label2: TLabel
      Left = 8
      Top = 48
      Width = 32
      Height = 13
      Caption = 'Group:'
    end
    object Panel3: TPanel
      Left = 0
      Top = 274
      Width = 239
      Height = 30
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 0
      object btnOK: TBitBtn
        Left = 84
        Top = 5
        Width = 75
        Height = 23
        Anchors = [akRight, akBottom]
        Caption = '&OK'
        Default = True
        ModalResult = 1
        TabOrder = 0
        OnClick = btnOKClick
      end
      object btnCancel: TBitBtn
        Left = 162
        Top = 5
        Width = 75
        Height = 23
        Anchors = [akRight, akBottom]
        Caption = '&Cancel'
        ModalResult = 2
        TabOrder = 1
      end
    end
    object edtCount: TEdit
      Left = 48
      Top = 24
      Width = 121
      Height = 21
      TabOrder = 1
      Text = '1'
      OnChange = edtCountChange
    end
    object rgrpType: TRadioGroup
      Left = 8
      Top = 128
      Width = 225
      Height = 57
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Type'
      ItemIndex = 0
      Items.Strings = (
        'Gun (Laser)'
        'Missile')
      TabOrder = 2
    end
    object rgrpDir: TRadioGroup
      Left = 8
      Top = 192
      Width = 225
      Height = 73
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Direction'
      ItemIndex = 0
      Items.Strings = (
        'Normal'
        'Anti-normal'
        'Specific:')
      TabOrder = 3
    end
    object edtDir: TEdit
      Left = 88
      Top = 240
      Width = 137
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 4
    end
    object rgrpPattern: TRadioGroup
      Left = 8
      Top = 80
      Width = 225
      Height = 41
      Caption = 'Pattern'
      ItemIndex = 0
      Items.Strings = (
        'Even distribution')
      TabOrder = 5
      OnClick = edtCountChange
    end
    object edtGroup: TEdit
      Left = 48
      Top = 48
      Width = 121
      Height = 21
      TabOrder = 6
      Text = '0'
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 333
    Height = 304
    Align = alClient
    BevelOuter = bvNone
    Caption = 'Panel2'
    TabOrder = 2
    object tbrMain: TToolBar
      Left = 0
      Top = 0
      Width = 333
      Height = 29
      EdgeBorders = []
      TabOrder = 0
      Visible = False
    end
    object Panel4: TPanel
      Left = 0
      Top = 29
      Width = 333
      Height = 275
      Align = alClient
      BevelOuter = bvNone
      Color = clWindow
      TabOrder = 1
      OnResize = Panel4Resize
      object View: TPaintBox
        Left = 0
        Top = 0
        Width = 333
        Height = 275
        Align = alClient
        OnPaint = ViewPaint
      end
    end
  end
end
