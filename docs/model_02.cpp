class CAnyPtr   //like boost::shared_ptr<void>
{
};

class CRecvHelper
{
    virtual size_t InitRecvSize() const = 0;
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const = 0;
    virtual std::pair<bool, CAnyPtr> HandleData(const char * buf, size_t sz) const = 0;
    virtual bool ProcessCmd(CAnyPtr cmdPtr, CSockSession & sock) = 0;
};

//-----------------------

class CCmdBase{};

class CCmdQuery : public CCmdBase{};

class CCmdResp : public CCmdBase{};

class CMyRecvHelper : public CRecvHelper
{
    __CmdQue & cmdQue_;
    CMyRecvHelper(__CmdQue & que):cmdQue_(que){}
    virtual size_t InitRecvSize() const;
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const;
    virtual std::pair<bool, CAnyPtr> HandleData(const char * buf, size_t sz, const CSockAddr & from, CSockSession & sock) const{
        CCmdBase * cmd = DecodeCmd(buf, sz);
        if(!cmd)
            return false;
        return std::make_pair(true, CAnyPtr(cmd));
    }
    virtual bool ProcessCmd(CAnyPtr cmdPtr, CSockSession & sock){
        CCmdQuery * cmd = PtrCast<CCmdBase *>(cmdPtr);  //ptr_cast + dynamic_cast
        if(!cmd)
            return false;
        //process cmd...
        CCmdResp resp(*cmd);
        std::string buf;
        resp.Encode(buf);
        sock.Send(buf);
        sock.SendTo(CSockAddr("1.2.3.4", 5678), buf);
    }
};

