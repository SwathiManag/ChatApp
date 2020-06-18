#include <wx/filename.h>
#include <wx/colour.h>
#include <wx/image.h>
#include <wx/timer.h>
#include <wx/msgdlg.h>
#include <thread>
#include <string>
#include "client.h"
#include "chatgui.h"

// size of chatbot window
const int width = 414;
const int height = 736;
#define TIMER_ID 100
// wxWidgets APP
IMPLEMENT_APP(ChatBotApp);

std::string dataPath = "../";
std::string imgBasePath = dataPath + "images/";
std::string host = "";

bool ChatBotApp::OnInit()
{
    // create window with name and show it
    if (wxApp::argc < 2) return false;
    host = std::string(wxApp::argv[1]);
    ChatBotFrame *chatBotFrame = new ChatBotFrame(wxT("ChatAppClient"));
    chatBotFrame->Show(true);

    return true;
}

BEGIN_EVENT_TABLE(ChatBotFrame, wxFrame)
EVT_CLOSE(ChatBotFrame::OnClose)
END_EVENT_TABLE()

// wxWidgets FRAME
ChatBotFrame::ChatBotFrame(const wxString &title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height))
{
    // create panel with background image
    ChatBotFrameImagePanel *ctrlPanel = new ChatBotFrameImagePanel(this);

    // create controls and assign them to control panel
    _panelDialog = new ChatBotPanelDialog(ctrlPanel, wxID_ANY);

    // create text control for user input
    int idTextXtrl = 1;
    _userTextCtrl = new wxTextCtrl(ctrlPanel, idTextXtrl, "", wxDefaultPosition, wxSize(width, 50), wxTE_PROCESS_ENTER, wxDefaultValidator, wxTextCtrlNameStr);
    Connect(idTextXtrl, wxEVT_TEXT_ENTER, wxCommandEventHandler(ChatBotFrame::OnEnter));

    // create vertical sizer for panel alignment and add panels
    wxBoxSizer *vertBoxSizer = new wxBoxSizer(wxVERTICAL);
    vertBoxSizer->AddSpacer(90);
    vertBoxSizer->Add(_panelDialog, 6, wxEXPAND | wxALL, 0);
    vertBoxSizer->Add(_userTextCtrl, 1, wxEXPAND | wxALL, 5);
    ctrlPanel->SetSizer(vertBoxSizer);

    // position window in screen center
    this->Centre();
}

void ChatBotFrame::OnEnter(wxCommandEvent &WXUNUSED(event))
{
    // retrieve text from text control
    wxString userText = _userTextCtrl->GetLineText(0);

    _panelDialog->Send(userText);

    // delete text in text control
    _userTextCtrl->Clear();
}

void ChatBotFrame::OnClose(wxCloseEvent& event){
    if (event.CanVeto())
    {
        if ( wxMessageBox("Continue closing?",
                          "Please confirm",
                          wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();
            return;
        }
    }
    _panelDialog->Close();
    Destroy(); 
}

BEGIN_EVENT_TABLE(ChatBotFrameImagePanel, wxPanel)
EVT_PAINT(ChatBotFrameImagePanel::paintEvent) // catch paint events
END_EVENT_TABLE()

ChatBotFrameImagePanel::ChatBotFrameImagePanel(wxFrame *parent) : wxPanel(parent)
{
}

void ChatBotFrameImagePanel::paintEvent(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void ChatBotFrameImagePanel::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}

void ChatBotFrameImagePanel::render(wxDC &dc)
{
    // load backgroud image from file
    wxString imgFile = imgBasePath + "sf_bridge.jpg";
    wxImage image;
    image.LoadFile(imgFile);

    // rescale image to fit window dimensions
    wxSize sz = this->GetSize();
    wxImage imgSmall = image.Rescale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH);
    _image = wxBitmap(imgSmall);
    
    dc.DrawBitmap(_image, 0, 0, false);
}

BEGIN_EVENT_TABLE(ChatBotPanelDialog, wxPanel)
EVT_PAINT(ChatBotPanelDialog::paintEvent) // catch paint events
EVT_TIMER(TIMER_ID, ChatBotPanelDialog::OnTimer)
END_EVENT_TABLE()

void ChatBotPanelDialog::ReceivingThread(){
    while(true){
        if(_clientSocket->recv(msg, size) <= 0){
            _timer.Stop();
            break;
        }
        else if (strncmp(msg, "CLOSEAPP", 9) == 0) {
            _clientSocket->send("CLOSEAPP", 9);
            break;
        }
        std::lock_guard<std::mutex> uLock(_mutex);
        _messageQueue.push_front(std::string(msg));
        // clearing message
        memset(msg, 0, 1024);
    }
}

void ChatBotPanelDialog::Send(wxString msg){
    std::string str_msg(msg);
    if (_clientSocket->send(str_msg.c_str(), str_msg.length()) != SOCKET_ERROR) {
        AddDialogItem(msg, true);
    }
    else {
        wxMessageBox(wxT("Error in sending the message"), wxT("ChatAppClient Error"), wxICON_ERROR);
    }
}

void ChatBotPanelDialog::OnTimer(wxTimerEvent& event)
{
    if (_messageQueue.size() > 0) {
        std::lock_guard<std::mutex> uLock(_mutex);
        AddDialogItem(wxString(_messageQueue.back()),false);
        _messageQueue.pop_back();
    }
}

void ChatBotPanelDialog::Close(){
    _timer.Stop();
    _clientSocket->send("CLOSEAPP", 9);
    _clientSocket->close(); //closing the socket
    if (_receiving_thread.joinable()) {
        _receiving_thread.join();
    }
}

ChatBotPanelDialog::ChatBotPanelDialog(wxWindow *parent, wxWindowID id)
    : wxScrolledWindow(parent, id), _timer(this, TIMER_ID)
{
    // sizer will take care of determining the needed scroll size
    _dialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(_dialogSizer);

    // allow for PNG images to be handled
    wxInitAllImageHandlers();

    _clientSocket = std::make_unique<SimpleTCPClientSocket>(); 

    // pass pointer to chatbot dialog so answers can be displayed in GUI
    if(!_clientSocket->connect(host.c_str())){
        wxMessageBox( wxT("Error in connecting to socket"), wxT("ChatAppClient error"), wxICON_ERROR);
        return;
    }
    _receiving_thread = std::thread{&ChatBotPanelDialog::ReceivingThread, this};
    _timer.Start(100);    // 100ms interval
}

ChatBotPanelDialog::~ChatBotPanelDialog()
{
    // close receiving thread gracefully
    if (_receiving_thread.joinable()) {
        _receiving_thread.join();
    }
}

void ChatBotPanelDialog::AddDialogItem(wxString text, bool isFromUser)
{
    // add a single dialog element to the sizer
    ChatBotPanelDialogItem *item = new ChatBotPanelDialogItem(this, text, isFromUser);
    _dialogSizer->Add(item, 0, wxALL | (isFromUser == true ? wxALIGN_RIGHT : wxALIGN_LEFT), 8);
    _dialogSizer->Layout();

    // make scrollbar show up
    this->FitInside(); // ask the sizer about the needed size
    this->SetScrollRate(5, 5);
    this->Layout();

    // scroll to bottom to show newest element
    int dx, dy;
    this->GetScrollPixelsPerUnit(&dx, &dy);
    int sy = dy * this->GetScrollLines(wxVERTICAL);
    this->DoScroll(0, sy);
}

void ChatBotPanelDialog::PrintChatbotResponse(std::string response)
{
    // convert string into wxString and add dialog element
    wxString botText(response.c_str(), wxConvUTF8);
    AddDialogItem(botText, false);
}

void ChatBotPanelDialog::paintEvent(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void ChatBotPanelDialog::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}

void ChatBotPanelDialog::render(wxDC &dc)
{
    wxImage image;
    image.LoadFile(imgBasePath + "sf_bridge_inner.jpg");

    wxSize sz = this->GetSize();
    wxImage imgSmall = image.Rescale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH);

    _image = wxBitmap(imgSmall);
    dc.DrawBitmap(_image, 0, 0, false);
}

ChatBotPanelDialogItem::ChatBotPanelDialogItem(wxPanel *parent, wxString text, bool isFromUser)
    : wxPanel(parent, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_NONE)
{
    // create image and text
    _chatBotTxt = new wxStaticText(this, wxID_ANY, text, wxPoint(-1, -1), wxSize(150, -1), wxALIGN_CENTRE | wxBORDER_NONE);
    _chatBotTxt->SetForegroundColour(isFromUser == true ? wxColor(*wxBLACK) : wxColor(*wxWHITE));

    // create sizer and add elements
    wxBoxSizer *horzBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    horzBoxSizer->Add(_chatBotTxt, 8, wxEXPAND | wxALL, 1);
    this->SetSizer(horzBoxSizer);

    // wrap text after 150 pixels
    _chatBotTxt->Wrap(150);

    // set background color
    this->SetBackgroundColour((isFromUser == true ? wxT("YELLOW") : wxT("BLUE")));
}
