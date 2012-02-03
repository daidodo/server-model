class CRecvHelper
{
    virtual size_t InitRecvSize() const = 0;
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const = 0;
    virtual bool HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const = 0;
    virtual void ReleaseCmd(const CAnyPtr & cmd) const{}
    virtual __Events ProcessCmd(const CAnyPtr & cmd, CSockSession & sock){return false;}
};

//-----------------------

class CCmdBase{};

class CCmdQuery : public CCmdBase{};

class CCmdResp : public CCmdBase{};

class CMyRecvHelper : public CRecvHelper
{
    virtual size_t InitRecvSize() const{return 5}
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const{return __Ret(COMPLETE, 0);}
    virtual bool HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const{
        CCmdBase * base = DecodeCmd(buf, sz);
        if(!base)
            return false;
        cmd = base;
        return true;
    }
    virtual void ReleaseCmd(const CAnyPtr & cmd) const{}
    virtual __Events ProcessCmd(const CAnyPtr & cmd, CSockSession & sock){
        CCmdBase * base = PtrCast<CCmdBase>(cmd);
        if(!cmd)
            return EVENT_CLOSE; //error
        std::string respdata;
        process(*base, respdata); //process cmd...
        if(!respdata.empty()){
            sock.Send(buf);
            return EVENT_OUT;   //output event
        }
        return 0;   //no event
    }
};

