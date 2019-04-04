#Boa:Frame:RFM12_GUI
import os
import binascii
import struct
import shutil
import wx
import wx.lib.filebrowsebutton

import pycurl
import cStringIO
import json
import sys


def create(parent):
    return RFM12_GUI(parent)

[wxID_RFM12_GUI, wxID_RFM12_GUIDELETEFILE, wxID_RFM12_GUIDESTINATION, 
 wxID_RFM12_GUIDUMPFILE, wxID_RFM12_GUIFILEBROWSEBUTTON1, 
 wxID_RFM12_GUIFILELISTBOX, wxID_RFM12_GUIGATEWAYRFM, wxID_RFM12_GUILOGS, 
 wxID_RFM12_GUIORDER, wxID_RFM12_GUISEND, wxID_RFM12_GUISENDFILE, 
 wxID_RFM12_GUISTATUS, wxID_RFM12_GUIVALUE, 
] = [wx.NewId() for _init_ctrls in range(13)]

class RFM12_GUI(wx.Frame):
    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_RFM12_GUI, name=u'RFM12_GUI',
              parent=prnt, pos=wx.Point(820, 169), size=wx.Size(704, 623),
              style=wx.DEFAULT_FRAME_STYLE, title=u'RFM12 GUI')
        self.SetClientSize(wx.Size(704, 623))
        self.SetAutoLayout(True)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnRFM12_GUIMouseEvents)

        self.send = wx.Button(id=wxID_RFM12_GUISEND, label=u'Send',
              name=u'send', parent=self, pos=wx.Point(448, 96),
              size=wx.Size(128, 48), style=0)
        self.send.Bind(wx.EVT_BUTTON, self.OnSendButton, id=wxID_RFM12_GUISEND)

        self.Order = wx.ListBox(choices=["RESET_BYTE ", " SET_BYTE",
              " RESET_BIT", "SET_BIT", "READ_BYTE", " READ_BIT", "ANALOG_READ",
              "ANSWER" , "SET_TIME", "GET_TIME", "SET_CHRONO", "SET_PROGRAMME",
              "RESET_EEPROM ", "SET_ID", " SET_TEMP_OFFSET", "GET_TEMP",
              " TIME_CALIBRATION" , "STOP_SERVER"], id=wxID_RFM12_GUIORDER,
              name=u'Order', parent=self, pos=wx.Point(144, 96),
              size=wx.Size(152, 168), style=0)
        self.Order.SetHelpText(u'Order send to target')
        self.Order.SetLabel(u'Order')
        self.Order.Bind(wx.EVT_LISTBOX, self.OnOrderListbox,
              id=wxID_RFM12_GUIORDER)

        self.Value = wx.TextCtrl(id=wxID_RFM12_GUIVALUE, name=u'Value',
              parent=self, pos=wx.Point(304, 96), size=wx.Size(144, 56),
              style=0, value=u'Must start x+ -')
        self.Value.SetHelpText(u'Couldbe x + - Value')

        self.Destination = wx.ListBox(choices=["2", "3", "4", "5", "6", "7",
              "8", "9", "10", "11", "30", "31"], id=wxID_RFM12_GUIDESTINATION,
              name=u'Destination', parent=self, pos=wx.Point(56, 96),
              size=wx.Size(64, 168), style=0)
        self.Destination.SetHelpText(u'target')
        self.Destination.SetLabel(u'Target')
        self.Destination.SetStringSelection(u'target')
        self.Destination.SetSelection(11)

        self.fileBrowseButton1 = wx.lib.filebrowsebutton.FileBrowseButton(buttonText=u'Browse',
              dialogTitle='Choose a file', fileMask=u'*',
              id=wxID_RFM12_GUIFILEBROWSEBUTTON1, initialValue='',
              labelText=u'Source', parent=self, pos=wx.Point(56, 8),
              size=wx.Size(520, 88), startDirectory='.', style=wx.TAB_TRAVERSAL,
              toolTip='Type filename or click browse to choose file')
        self.fileBrowseButton1.SetLabel(u'Source')
        self.fileBrowseButton1.SetValue(u'')
        self.fileBrowseButton1.SetAutoLayout(True)
        self.fileBrowseButton1.Bind(wx.EVT_KEY_UP,
              self.OnFileBrowseButton1KeyUp)

        self.Status = wx.TextCtrl(id=wxID_RFM12_GUISTATUS, name=u'Status',
              parent=self, pos=wx.Point(56, 272), size=wx.Size(512, 136),
              style=wx.TE_MULTILINE, value=u'Status')
        self.Status.SetAutoLayout(True)

        self.Logs = wx.TextCtrl(id=wxID_RFM12_GUILOGS, name=u'Logs',
              parent=self, pos=wx.Point(56, 416), size=wx.Size(512, 168),
              style=wx.TE_MULTILINE, value=u'Logs')
        self.Logs.Bind(wx.EVT_SET_FOCUS, self.OnLogsSetFocus)

        self.fileListBox = wx.ListBox(choices=["", " "],
              id=wxID_RFM12_GUIFILELISTBOX, name=u'fileListBox', parent=self,
              pos=wx.Point(312, 160), size=wx.Size(168, 104), style=0)
        self.fileListBox.SetStringSelection(u'')
        self.fileListBox.SetSelection(0)
        self.fileListBox.Bind(wx.EVT_SET_FOCUS, self.OnFileListBoxSetFocus)

        self.SendFile = wx.Button(id=wxID_RFM12_GUISENDFILE, label=u'SendFile',
              name=u'SendFile', parent=self.fileBrowseButton1, pos=wx.Point(432,
              56), size=wx.Size(80, 29), style=0)
        self.SendFile.Bind(wx.EVT_BUTTON, self.OnSendFileButton,
              id=wxID_RFM12_GUISENDFILE)

        self.gateWayRFM = wx.TextCtrl(id=wxID_RFM12_GUIGATEWAYRFM,
              name=u'gateWayRFM', parent=self.fileBrowseButton1,
              pos=wx.Point(160, 8), size=wx.Size(144, 24), style=0,
              value=u'192.168.0.10')

        self.deleteFile = wx.Button(id=wxID_RFM12_GUIDELETEFILE,
              label=u'deleteFile', name=u'deleteFile', parent=self,
              pos=wx.Point(488, 176), size=wx.Size(85, 29), style=0)
        self.deleteFile.Bind(wx.EVT_BUTTON, self.OnDeleteFileButton,
              id=wxID_RFM12_GUIDELETEFILE)

        self.dumpFile = wx.Button(id=wxID_RFM12_GUIDUMPFILE, label=u'dumpFile',
              name=u'dumpFile', parent=self, pos=wx.Point(488, 216),
              size=wx.Size(85, 32), style=0)
        self.dumpFile.Bind(wx.EVT_BUTTON, self.OnDumpFileButton,
              id=wxID_RFM12_GUIDUMPFILE)

    def __init__(self, parent):
        self._init_ctrls(parent)


#detection de la commande 
    def OnOrderListbox(self, event):
        self.Value.SetValue("") #to be sure nothing in Value
        
        event.Skip()


    def OnSendButton(self, event):
        lOrder=self.Order.GetSelection() 
        slOrder = self.Order.GetString(self.Order.GetSelection()) 
        lSource = self.fileBrowseButton1.GetValue()
        slFile = self.fileListBox.GetString(self.fileListBox.GetSelection())
        
        print 'Order '+str(lOrder)
        if lOrder == -1 :
            self.Status.SetValue("Selectionner un Ordre") 
            #else : 
            #self.Status.SetValue(str(lOrder))
        else :   
            lDestination =self.Destination.GetSelection()
            if lDestination == -1 : 
               self.Status.SetValue("Selectionner une cible")
            #else :
            #  self.Status.SetValue(self.Destination.GetString(lDestination))            
            else : 
                statif=0    # Value  1 is for ok to send
                lValue=""
                llValue = ""
                lValue= self.Value.GetLineText(0)
                if lValue == "":
                    llValue = ""
                    self.Status.SetValue("Selectionner une valeur")
                    statif=1
               
                elif lValue[0] == '-' :
                        a = int(lValue[0:]);
                        a = int(a)
                        print 'a '+str(a)
                        llValue = "{0:04X}".format(a & 0xFFFF,'016b')
                        self.Status.SetValue("Sub Ofsett Temperature"+lValue)
                        statif=1
                elif lValue[0] == '+' :
                        a= int(lValue);
                        print 'a '+str(a)
                        
                        llValue = "{0:04X}".format(a & 0xFFFF,'016b')
                        self.Status.SetValue("Add Ofsett Temperature"+lValue)
                        statif=1
                elif lValue[0] == 'x' :
                        print len(lValue)
                        if len(lValue) == 2 :
                            llValue = '0'+lValue[1:]
                        else :
                            llValue = lValue[1:]

                        self.Status.SetValue(lValue) 
                        statif=1
                else :
                        self.Status.SetValue("value error Must start by +- or x")         
                        event.Skip()
                
                    
                    
                if  ( slOrder == "SET_CHRONO" or \
                      slOrder == "SET_PROGRAMME" ):       
                        if slFile == "" :
                            statif= 0
                            self.Status.SetValue("File Source must be defined")    
                        else  : # Copy and rename  source file in destinantion 
                            statif= 1
                else :
                     lSource =""            
                if  ( slOrder == "STOP_SERVER" ):  
                    lOrder = ord('0')
                    statif= 1                            
                            
                if statif== 1 :  
                    record = ""
                    print 'lOrder'+str(lOrder)
                    print 'llValue'+llValue
#str(chr(00)+chr(int(self.Destination.GetString(lDestination)))+chr(lOrder)+llValue+'\n')
                    record =  ByteToHex(chr(00))+ByteToHex(chr(int(self.Destination.GetString(lDestination))))+ByteToHex(chr(lOrder))+llValue
                    self.Status.SetValue("Send "+record)
                    
                    lHost = self.gateWayRFM.GetValue() 
                    curlBuf = cStringIO.StringIO()
                    oCurl = pycurl.Curl()
#send order: http://192.168.0.10/order?file=/bibliotheque&order=031E0A00"     
                    if slFile == "" :
                        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/order?order='+record)
                        print record        
                    else :   
                        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/order?order='+record+'&file=/'+slFile) 
                        print record +' file '+ slFile                      
                    oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
                    oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
                    oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
                    oCurl.perform()
                    Rfile = curlBuf.getvalue()
                    if Rfile != "" :
                        self.Status.SetValue(Rfile)        
#        print buf.getvalue()
                    curlBuf.close() 
                    self.refresh_all()
                    event.Skip()

#-------------------------------------------------------------------------------

    def OnLogsSetFocus(self, event):
#        curl 192.168.0.10/get
        self.refresh_all()
        event.Skip()
#-------------------------------------------------------------------------------


    def OnFileListBoxSetFocus(self, event):

        lHost = self.gateWayRFM.GetValue() 
        curlBuf = cStringIO.StringIO()
        oCurl = pycurl.Curl()
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/list?dir=/') 
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()
        Rfile = json.loads(curlBuf.getvalue())
        
        self.fileListBox.Set([""]) 
        for Efile in Rfile : 
            print Efile ['name'] 
            self.fileListBox.Append(Efile ['name'] ) 
    
#        print buf.getvalue()
        curlBuf.close()
        event.Skip()
#-------------------------------------------------------------------------------

    def OnFileBrowseButton1KeyUp(self, event):
        lHost = self.gateWayRFM.GetValue()
        lSource = self.fileBrowseButton1.GetValue()
        print lSource
        curlBuf = cStringIO.StringIO()
 
        oCurl = pycurl.Curl()
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/list?dir=/')        
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()
        Rfile = json.loads(curlBuf.getvalue())
        
        self.fileListBox.Set([""]) 
        for Efile in Rfile : 
            print Efile ['name'] 
            self.fileListBox.Append(Efile ['name'] ) 
    
#        print buf.getvalue()
        curlBuf.close()  
        event.Skip()
#-------------------------------------------------------------------------------

    def OnDeleteFileButton(self, event):
        
        lHost = self.gateWayRFM.GetValue()
        nFile =os.path.basename(lHost)
        slFile = self.fileListBox.GetString(self.fileListBox.GetSelection())        
        curlBuf = cStringIO.StringIO()
        oCurl = pycurl.Curl()
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/fileDelete?file=/'+slFile)
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()    
#        print buf.getvalue()
        curlBuf.close()  

        event.Skip()
#-------------------------------------------------------------------------------

    def OnDumpFileButton(self, event):
        lHost = self.gateWayRFM.GetValue()
        nFile =os.path.basename(lHost)
        slFile = self.fileListBox.GetString(self.fileListBox.GetSelection())
        curlBuf = cStringIO.StringIO()
        oCurl = pycurl.Curl()       
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/fileRead?file=/'+slFile)
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()
# Get content File
        self.Status.SetValue(curlBuf.getvalue())
     
    
#        print buf.getvalue()
        curlBuf.close()  

        event.Skip()
#-------------------------------------------------------------------------------

    def OnSendFileButton(self, event):
        lSource = self.fileBrowseButton1.GetValue()
        print lSource
        lHost = self.gateWayRFM.GetValue()
        nFile =os.path.basename(lHost)
        print(nFile)
    
        curlBuf = cStringIO.StringIO()
        oCurl = pycurl.Curl()
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/file')
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
        oCurl.setopt(oCurl.HTTPPOST, [(nFile, (oCurl.FORM_FILE,lSource))])
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()
#        print buf.getvalue()
        curlBuf.close()  
        event.Skip()

    def OnRFM12_GUIMouseEvents(self, event):
        event.Skip()
        
    def refresh_all(self):
        print 'getlogs_F'
        lHost = self.gateWayRFM.GetValue() 
        curlBuf = cStringIO.StringIO()
        oCurl = pycurl.Curl()
        oCurl.setopt(oCurl.URL, 'http://'+lHost+'/get')                        
        oCurl.setopt(oCurl.WRITEFUNCTION, curlBuf.write)
        oCurl.setopt(oCurl.CONNECTTIMEOUT, 5)
        oCurl.setopt(oCurl.TIMEOUT, 30)
#        oCurl.setopt(oCurl.PROXY, 'http://inthemiddle.com:8080')
        oCurl.perform()
        Rfile = curlBuf.getvalue()
        if Rfile != "" :
             self.Logs.AppendText(Rfile)
        print 'logs='+Rfile
#        print buf.getvalue()
        curlBuf.close() 
        return
def ByteToHex( byteStr ):
    """
    Convert a byte string to it's hex string representation e.g. for output.
    """
    
    # Uses list comprehension which is a fractionally faster implementation than
    # the alternative, more readable, implementation below
    #   
    #    hex = []
    #    for aChar in byteStr:
    #        hex.append( "%02X " % ord( aChar ) )
    #
    #    return ''.join( hex ).strip()        

    return ''.join( [ "%02X " % ord( x ) for x in byteStr ] ).strip()

#-------------------------------------------------------------------------------

def HexToByte( hexStr ):
    """
    Convert a string hex byte values into a byte string. The Hex Byte values may
    or may not be space separated.
    """
    # The list comprehension implementation is fractionally slower in this case    
    #
    #    hexStr = ''.join( hexStr.split(" ") )
    #    return ''.join( ["%c" % chr( int ( hexStr[i:i+2],16 ) ) \
    #                                   for i in range(0, len( hexStr ), 2) ] )
 
    bytes = []

    #hexStr = ''.join( hexStr.split(" ") )


    for i in range(0, len(hexStr), 2):
        bytes.append( chr( int (hexStr[i:i+2], 16 ) ) )

    return ''.join( bytes )

#-------------------------------------------------------------------------------

def decToHex( decStr ):
    """
    Convert a string Decimal byte values into a Hexa. The Hex Byte values may
    or may not be space separated.
    """
    # The list comprehension implementation is fractionally slower in this case    
    #
    #    hexStr = ''.join( hexStr.split(" ") )
    #    return ''.join( ["%c" % chr( int ( hexStr[i:i+2],16 ) ) \
    #                                   for i in range(0, len( hexStr ), 2) ] )
 
    bytes = []

    decStr = ''.join( decStr.split(" ") )

    for i in range(0, len(decStr), 2):
        bytes.append( chr( int (decStr[i:i+2], 16 ) ) )

    return ''.join( bytes )



#-------------------------------------------------------------------------------
