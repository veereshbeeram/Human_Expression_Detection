'''
Created on Jan 6, 2011

@author: veeresh Beeram
@organization: Department of Information Technology, NITK surathkal. 
@copyright: Towards B.Tech Major Project requirments 
@since: july 2010
'''

'''
@summary: This is a stub code written in python, using wxPython GUI toolkit, towards setting correct 
        parameters to run an opencv code written in C. The aim of opencv code is to do human emotion
        recognition.

'''
import wx
import os




class MyApp(wx.App):
    def OnInit(self):
        self.frame = MyFrame(None,title="Emotion Recognition")
        self.SetTopWindow(self.frame)
        self.frame.Show()
        
        return True
    
class MyFrame(wx.Frame):
    def __init__(self,parent,id=wx.ID_ANY,title="",pos=wx.DefaultPosition,size =(500,500),
                 style=wx.DEFAULT_FRAME_STYLE,name="MyFrame"):
        super(MyFrame,self).__init__(parent,id,title,pos,size,style,name)
        
        #vars to store filenames
        self.basepath = ""
        self.emotionpath = ""
        self.videopath = ""
        
        #Attributes
        self.mainpanel = wx.Panel(self)
        self.path = os.path.abspath("./smile.jpg")
        self.iconimg = wx.Icon(self.path,wx.BITMAP_TYPE_JPEG)
        self.SetIcon(self.iconimg)
        

        self.choiceList = ["Static Images","Video On File","Live Camera feed"]
        self.radiobox = wx.RadioBox(self.mainpanel,label="Choose Input Type",choices=self.choiceList,
                               style=wx.RA_SPECIFY_ROWS|wx.RA_VERTICAL)
        
        # File selection Dialog trigger buttons
        self.videoBtn = wx.Button(self.mainpanel,label="Select Video File")
        self.baseimgBtn = wx.Button(self.mainpanel,label="Select Base Image")
        self.emotionimgBtn = wx.Button(self.mainpanel,label="Select Emotion Image")
        
        # File selection Dialog texts
        self.basefile = wx.StaticText(self.mainpanel,label="Choose Base Image File")
        self.emotionfile = wx.StaticText(self.mainpanel,label="Choose Emotion Image File")
        self.videofile = wx.StaticText(self.mainpanel,label="Choose Video File")
        
        self.verbose = wx.CheckBox(self.mainpanel,label="Verbose Mode")
        self.okbtn = wx.Button(self.mainpanel,wx.ID_OK)
        
        #event Handlers for all buttons
        self.Bind(wx.EVT_BUTTON, self.OnbaseimgBtn, self.baseimgBtn)
        self.Bind(wx.EVT_BUTTON, self.OnemotionimgBtn, self.emotionimgBtn)
        self.Bind(wx.EVT_BUTTON, self.OnvideoBtn, self.videoBtn)
        self.Bind(wx.EVT_BUTTON, self.Onokbtn, self.okbtn)
        
        #base image Hbox
        self.hboxbaseimg = wx.BoxSizer(wx.HORIZONTAL)
        self.hboxbaseimg.Add(self.basefile, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.hboxbaseimg.Add(self.baseimgBtn, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        #emotion image Hbox
        self.hboxemotionimg = wx.BoxSizer(wx.HORIZONTAL)
        self.hboxemotionimg.Add(self.emotionfile, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.hboxemotionimg.Add(self.emotionimgBtn, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        #video file Hbox
        self.hboxvideo = wx.BoxSizer(wx.HORIZONTAL)
        self.hboxvideo.Add(self.videofile, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.hboxvideo.Add(self.videoBtn, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        #confirm hbox
        self.hboxfoot = wx.BoxSizer(wx.HORIZONTAL)
        self.hboxfoot.Add(self.verbose, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.hboxfoot.Add(self.okbtn, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        #Vbox for Radio group
        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.radiobox, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        # Main Vbox
        self.ctrlvbox = wx.BoxSizer(wx.VERTICAL)
        self.ctrlvbox.Add(self.vbox, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.ctrlvbox.Add(self.hboxbaseimg, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.ctrlvbox.Add(self.hboxemotionimg, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.ctrlvbox.Add(self.hboxvideo, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        self.ctrlvbox.Add(self.hboxfoot, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        # self.ctrlvbox.Add(self.okbtn, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        
        self.mainpanel.SetSizer(self.ctrlvbox)  
     
    def OnbaseimgBtn(self,event):
        self.getTheName("base")
    def OnemotionimgBtn(self,event):
        self.getTheName("emotion")
    def OnvideoBtn(self,event):
        self.getTheName("video")
    
    def Onokbtn(self,event):
        isrunnable =1
        execCommand = "./imageProcessing"
        verMode = 0
        if self.verbose.IsChecked():
            print "verbose checked"
            verMode = 1
        inType = self.radiobox.Selection
        execCommand = execCommand + " " + str(verMode) + " " + str(inType)+ " "+ "1"
        if inType == 0:
            if (len(self.basepath)<5 or len(self.emotionpath) < 5):
                wx.MessageBox("Select both base and emotion image")
                isrunnable = 0
            execCommand =  execCommand + " \"" + self.basepath + "\" \"" + self.emotionpath + "\""
        elif inType == 1:
            if len(self.videopath)<5:
                wx.MessageBox("Select video file")
                isrunnable = 0
            execCommand =  execCommand + " \"" + self.videopath + "\""
        else:
            #need to call camera check here
            retval = os.system("./cam")
            if(retval!=0):
                wx.MessageBox("camera detect failed")
                isrunnable = 0
            else:
                print "camera called "+ str(retval)
        
        
        if(isrunnable):
            os.system(execCommand);
        
        return
        
    def getTheName(self,whatType):
        wildcard = "JPEG Image or AVI video Files (*.jpg, *.avi)|*.jpg;*.avi;*.JPG;*.AVI"
        dlg = wx.FileDialog(self,message="Open A File", wildcard = wildcard,style = wx.FD_OPEN)
        if dlg.ShowModal():
            tmpPath = dlg.GetPath()
        dlg.Destroy()
        oriPath = tmpPath
        tmpPath = str(tmpPath)
        tmparr = tmpPath.split("/")
        tmpPath = "./" + tmparr[-2] + "/" + tmparr[-1]
        print tmpPath
        if whatType == "base":
            self.basepath = oriPath
            self.basefile.SetLabel(tmpPath)
        elif whatType == "emotion":
            self.emotionpath = oriPath
            self.emotionfile.SetLabel(tmpPath)
        else:
            self.videopath = oriPath
            self.videofile.SetLabel(tmpPath)
        return
               
    
 
    
if __name__ == "__main__":
    myapp = MyApp(redirect=False)
    myapp.MainLoop()

