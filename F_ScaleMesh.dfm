object ScaleMeshForm: TScaleMeshForm
  Left = 508
  Top = 305
  AutoScroll = False
  BorderIcons = []
  Caption = 'Scale Selected Meshes'
  ClientHeight = 300
  ClientWidth = 392
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
  object Bevel1: TBevel
    Left = 8
    Top = 104
    Width = 376
    Height = 9
    Anchors = [akLeft, akTop, akRight]
    Shape = bsTopLine
  end
  object Label1: TLabel
    Left = 8
    Top = 224
    Width = 73
    Height = 13
    Caption = 'Scale Factor'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lvwMeshes: TListView
    Left = 8
    Top = 8
    Width = 377
    Height = 89
    Anchors = [akLeft, akTop, akRight]
    Columns = <
      item
        Caption = 'Mesh Name'
        MinWidth = 20
        Width = 125
      end
      item
        Caption = 'X-Span'
        MinWidth = 10
        Width = 75
      end
      item
        Caption = 'Y-Span'
        MinWidth = 10
        Width = 75
      end
      item
        Caption = 'Z-Span'
        MinWidth = 10
        Width = 75
      end>
    ColumnClick = False
    HideSelection = False
    ReadOnly = True
    RowSelect = True
    TabOrder = 0
    ViewStyle = vsReport
  end
  object StatusBar1: TStatusBar
    Left = 368
    Top = 281
    Width = 24
    Height = 19
    Align = alNone
    Anchors = [akRight, akBottom]
    Panels = <
      item
        Bevel = pbNone
        Width = 50
      end>
    SimplePanel = False
  end
  object rgpScaleSys: TRadioGroup
    Left = 8
    Top = 112
    Width = 169
    Height = 57
    Caption = 'Scaling system'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Items.Strings = (
      'Percentile'
      'Make absolute size')
    ParentFont = False
    TabOrder = 2
  end
  object chkScaleOffs: TCheckBox
    Left = 8
    Top = 176
    Width = 169
    Height = 17
    Caption = 'Scale offsets to children'
    TabOrder = 3
  end
  object edtScaleFactor: TEdit
    Left = 24
    Top = 240
    Width = 360
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 4
    Text = '<1, 1, 1>'
  end
  object Panel1: TPanel
    Left = 184
    Top = 112
    Width = 200
    Height = 105
    Anchors = [akLeft, akTop, akRight]
    BevelInner = bvLowered
    TabOrder = 5
    object Label2: TLabel
      Left = 2
      Top = 2
      Width = 196
      Height = 101
      Align = alClient
      AutoSize = False
      Caption = 
        'The meshes listed above will be scaled by the "Scale Factor" abo' +
        'ut its center.  This number is interpretted according to the sel' +
        'ection "Scaling system."  If the mesh contains children, then it' +
        ' is recommended that you select "Scale offsets."'
      WordWrap = True
    end
  end
  object BitBtn1: TBitBtn
    Left = 216
    Top = 273
    Width = 75
    Height = 23
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
  end
  object BitBtn2: TBitBtn
    Left = 296
    Top = 273
    Width = 75
    Height = 23
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 7
  end
end
