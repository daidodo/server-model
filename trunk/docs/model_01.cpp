class CRecvHelper
{
    virtual size_t InitRecvSize() const = 0;
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const = 0;
    virtual bool HandleData(const char * buf, size_t sz, const CSockAddr & from, CSockSession & sock) const = 0;
};

//-----------------------

class CCmdBase{};

class CCmdQuery : public CCmdBase{};

class CCmdResp : public CCmdBase{};

class CMyRecvHelper : public CRecvHelper
{
    virtual size_t InitRecvSize() const;
    virtual __Ret OnDataArrive(const char * buf, size_t sz) const;
    virtual bool HandleData(const char * buf, size_t sz, const CSockAddr & from, CSockSession & sock) const{
        CInByteStream in(buf, sz);
        CCmdQuery cmd;
        if(!(in>>cmd))
            return false;
        CCmdResp resp(cmd)
        if(!process(cmd, resp))
            return false;
        COutByteStream out;
        out<<resp;
        std::string buf;
        out.ExportData(buf);
        sock.Send(buf);
        return true;
    }
};
