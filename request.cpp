#include "request.hpp"

#include "def/debug_def.hpp"
#include "utilities/string_utilities.hpp"
#include "utilities/file_utilities.hpp"

#include <sstream>

namespace da4qi4
{

MultiPart::MultiPart(MultiPart const& o)
    : _headers(o._headers), _data(o._data)
{
}

MultiPart::MultiPart(MultiPart&& o)
    : _headers(std::move(o._headers)), _data(std::move(o._data))
{
}

bool MultiPart::IsExistsHeader(std::string const& field) const
{
    return Utilities::IsExistsHeader(_headers, field);
}

std::string const& MultiPart::GetHeader(std::string const& field) const
{
    return Utilities::GetHeader(_headers, field);
}

OptionalStringRefConst MultiPart::TryGetHeader(std::string const& field) const
{
    return Utilities::TryGetHeader(_headers, field);
}

void MultiPart::AppendHeader(std::string&& field, std::string&& value)
{
    auto it = _headers.find(field);

    if (it != _headers.end())
    {
        it->second = std::move(value);
    }
    else
    {
        _headers.insert(std::make_pair(std::move(field), std::move(value)));
    }
}

bool Request::IsExistsHeader(std::string const& field) const
{
    return Utilities::IsExistsHeader(_headers, field);
}

std::string const& Request::GetHeader(std::string const& field) const
{
    return Utilities::GetHeader(_headers, field);
}

OptionalStringRefConst Request::TryGetHeader(std::string const& field) const
{
    return Utilities::TryGetHeader(_headers, field);
}

bool Request::IsExistsUrlParameter(std::string const& name) const
{
    return Utilities::IsExistsHeader(_url.parameters, name);
}

std::string const& Request::GetUrlParameter(std::string const& name) const
{
    return Utilities::GetHeader(_url.parameters, name);
}

OptionalStringRefConst Request::TryGetUrlParameter(std::string const& name) const
{
    return Utilities::TryGetHeader(_url.parameters, name);
}

bool Request::IsExistsCookie(std::string const& name) const
{
    return Utilities::IsExistsHeader(_cookies, name);
}

std::string const& Request::GetCookie(std::string const& name) const
{
    return Utilities::GetHeader(_cookies, name);
}

OptionalStringRefConst Request::TryGetCookie(std::string const& name) const
{
    return Utilities::TryGetHeader(_cookies, name);
}

bool Request::IsExistsFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return true;
        }
    }

    return false;
}

std::string const& Request::GetFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return fd.data;
        }
    }

    return Utilities::theEmptyString;
}

OptionalStringRefConst Request::TryGetFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return OptionalStringRefConst(fd.data);
        }
    }

    return OptionalStringRefConst(NoneObject);
}

std::string const& Request::GetParameter(std::string const& name) const
{
    ParameterSrc src = IsExistsParameter(name);

    switch (src)
    {
        case fromUrl:
            return GetUrlParameter(name);

        case fromForm:
            return GetFormData(name);

        case fromHeader:
            return GetHeader(name);

        case fromCookie:
            return GetCookie(name);

        case fromUnknown:
            break;
    }

    return Utilities::theEmptyString;
}

OptionalStringRefConst Request::TryGetParameter(std::string const& name) const
{
    ParameterSrc src = IsExistsParameter(name);

    switch (src)
    {
        case fromUrl:
            return TryGetUrlParameter(name);

        case fromForm:
            return TryGetFormData(name);

        case fromHeader:
            return TryGetHeader(name);

        case fromCookie:
            return TryGetCookie(name);

        case fromUnknown:
            break;
    }

    return OptionalStringRefConst(NoneObject);
}

bool GetURLPartValue(int url_part_flag,  Url& url, std::string&& value)
{
    switch (url_part_flag)
    {
        case UF_SCHEMA :
            url.schema.swap(value);
            break;

        case UF_HOST :
            url.host.swap(value);
            break;

        case UF_PORT:
            break; //skip, but return true;

        case UF_PATH :
            url.path.swap(value);
            break;

        case UF_QUERY :
            url.query.swap(value);
            break;

        case UF_FRAGMENT :
            url.fragment.swap(value);
            break;

        case UF_USERINFO :
            url.userinfo.swap(value);
            break;

        default:
            return false;
    }

    return true;
}

bool Request::ParseUrl(std::string&& url)
{
    http_parser_url r;
    http_parser_url_init(&r);
    int err = http_parser_parse_url(url.c_str(), url.length(), 0, &r);

    if (0 == err)
    {
        _url.port = r.port;

        for (unsigned int i = 0; i < UF_MAX; ++i)
        {
            if ((r.field_set & (1 << i)) == 0)
            {
                continue;
            }

            GetURLPartValue(i, _url, std::string(url.c_str() + r.field_data[i].off, r.field_data[i].len));
        }
    }

    _url.full.swap(url);
    return !err;
}


void Request::AppendHeader(std::string&& field, std::string&& value)
{
    auto it = _headers.find(field);

    if (it != _headers.end())
    {
        it->second = std::move(value);
    }
    else
    {
        _headers.insert(std::make_pair(std::move(field), std::move(value)));
    }
}

void Request::Reset()
{
    _url.Clear();
    _method = HTTP_GET;
    _headers.clear();
    _version_major = _version_minor = 0;
    _content_length = 0;
    _flags = 0;
    _addition_flags.reset();

    if (_body.length() < 1024 * 10)
    {
        _body.clear();
    }
    else
    {
        std::string tmp;
        _body.swap(tmp);
        _body.reserve(1024 * 2);
    }

    _boundary.clear();
    _multiparts.clear();
    _cookies.clear();
    _formdata.clear();
}

void Request::ParseContentType()
{
    auto content_type = this->TryGetHeader("Content-Type");

    if (content_type)
    {
        auto beg = content_type->find("multipart/");

        if (beg != std::string::npos)
        {
            constexpr int len_of_multipart_flag = 10; // length of "multipart/"
            constexpr int len_of_boundary_flag = 9;   // length of "boundary="
            this->MarkMultiPart(true);

            if (content_type->find("form-data", beg + len_of_multipart_flag) != std::string::npos)
            {
                this->MarkFormData(true);
            }

            auto pos = content_type->find("boundary=", beg + len_of_multipart_flag);

            if (pos != std::string::npos)
            {
                _boundary = content_type->substr(pos + len_of_boundary_flag);
            }
        }
    }
}

void Request::SetMultiPartBoundary(char const* at, size_t length)
{
    _boundary = std::string(at, length);
}

void MultiPartSplitSubHeadersFromValue(std::string const& value, MultiPart::SubHeaders& sub_headers)
{
    std::vector<std::string> parts = Utilities::SplitByChar(value, ';');
    sub_headers.value.clear();

    for (auto p : parts)
    {
        auto pos = p.find('=');

        if (pos == std::string::npos)
        {
            if (sub_headers.value.empty())
            {
                sub_headers.value = p;
            }
        }
        else
        {
            std::string f = p.substr(0, pos);
            Utilities::Trim(f);
            std::string v = p.substr(pos + 1);
            Utilities::Trim(v);
            size_t vl = v.size();

            if (vl >= 2)
            {
                if (v[0] == '\"' && v[vl - 1] == '\"')
                {
                    v = v.substr(1, vl - 2);
                }
            }

            if (!f.empty())
            {
                sub_headers.headers.emplace(std::move(f), std::move(v));
            }
        }
    }
}

MultiPart::SubHeaders MultiPart::GetSubHeaders(std::string const& field)
{
    SubHeaders h;
    std::string const& value = this->GetHeader(field);

    if (!value.empty())
    {
        MultiPartSplitSubHeadersFromValue(value, h);
    }

    return h;
}

void Request::TransferHeadersToCookies()
{
    auto value = GetHeader("Cookie");

    if (!value.empty())
    {
        std::vector<std::string> parts = Utilities::SplitByChar(value, ';');

        for (auto part : parts)
        {
            std::vector<std::string> kv = Utilities::SplitByChar(part, '=');

            if (!kv.empty())
            {
                std::string const& k = kv[0];
                std::string const& v = (kv.size() > 1) ? kv[1] : Utilities::theEmptyString;
                _cookies.emplace(std::move(k), std::move(v));
            }
        }
    }
}

bool TransferMultiPartToFormDataItem(MultiPart&& mp, FormDataItem& fdi)
{
    fdi.Reset();
    MultiPart::SubHeaders sub_headers = mp.GetSubHeaders("Content-Disposition");

    if (sub_headers.IsEmpty() || !Utilities::iEquals(sub_headers.value, "form-data"))
    {
        return false;
    }

    std::string name = Utilities::GetHeader(sub_headers.headers, "name");
    auto content_type = mp.TryGetHeader("Content-Type");

    if (name.empty())
    {
        return false;
    }

    fdi.name = name;
    auto filename = Utilities::TryGetHeader(sub_headers.headers, "filename");

    if (filename)
    {
        fdi.data_flag = FormDataItem::is_file_data;
        fdi.filename = *filename;
    }

    if (content_type)
    {
        fdi.content_type = *content_type;
    }

    fdi.data = std::move(mp.GetData());
    return true;
}

std::string MakeUploadFileTemporaryName(std::string const& ext)
{
    boost::uuids::uuid uid;
    std::stringstream ss;
    ss << uid << ext;
    return ss.str();
}

void Request::TransferMultiPartsToFormData(UploadFileSaveOptions const& options, std::string const& dir)
{
    std::vector<MultiPart> result_multiparts;

    for (size_t i = 0; i < _multiparts.size(); ++i)
    {
        MultiPart& part = _multiparts[i];
        FormDataItem fd_item;

        if (TransferMultiPartToFormDataItem(std::move(part), fd_item))
        {
            if (!dir.empty() && fd_item.data_flag == FormDataItem::is_file_data
                && !fd_item.filename.empty() && !fd_item.data.empty())
            {
                std::string ext = fs::extension(fd_item.filename); //ext maybe empty
                size_t filesize_kb = fd_item.data.size() / 1024;

                if (options.IsNeedSave(ext, filesize_kb))
                {
                    std::string filename = MakeUploadFileTemporaryName(ext);

                    if (Utilities::SaveDataToFile(fd_item.data, dir, filename))
                    {
                        fd_item.data = filename;
                        fd_item.data_flag = FormDataItem::is_file_temporary_name;
                    }
                }
            }

            _formdata.push_back(std::move(fd_item));
        }
        else
        {
            result_multiparts.push_back(std::move(part));
        }
    }

    _multiparts = std::move(result_multiparts);
}


std::string Request::dump() const
{
    std::stringstream ss;
    ss << "====== URL ======\r\n";
    ss << "url : " << _url.full <<  "\r\n";
    ss << "host : " << _url.host <<  "\r\n";
    ss << "port : " << _url.port <<  "\r\n";
    ss << "path : " << _url.path <<  "\r\n";
    ss << "query : " << _url.query <<  "\r\n";
    ss << "fragment : " << _url.fragment <<  "\r\n";
    ss << "userinfo : " << _url.userinfo <<  "\r\n";
    ss << "====== URL PARAMETERS ======\r\n";

    for (auto const& p : _url.parameters)
    {
        ss << p.first << " : " << p.second <<  "\r\n";
    }

    ss << "====== METHOD ======\r\n";
    ss << "method : " << this->GetMethodName() <<  "\r\n";
    ss << "====== HEADERS ======\r\n";

    for (auto const& p : _headers)
    {
        ss << p.first << " : " << p.second << "\r\n";
    }

    ss << "====== FLAGS ======\r\n";
    ss << "upgrade : " << std::boolalpha << this->IsUpgrade() << "\r\n";
    ss << "has content-length : " << std::boolalpha << this->IsContentLengthProvided() << "\r\n";
    ss << "chunked : " << std::boolalpha << this->IsChunked() << "\r\n";
    ss << "multipart : " << std::boolalpha << this->IsMultiPart() << "\r\n";
    ss << "formdata : " << std::boolalpha << this->IsFormData() << "\r\n";
    ss << "keepalive : " << std::boolalpha << this->IsKeepAlive() << "\r\n";

    if (this->IsMultiPart())
    {
        ss << "boundary : " << GetMultiPartBoundary() << "\r\n";
    }

    ss << "====== BODY ======\r\n";
    ss << _body <<  "\r\n";
    ss << "======MULTIPART======\r\n";

    for (auto const& mp : this->_multiparts)
    {
        for (auto const& h : mp.GetHeaders())
        {
            ss << h.first << " = " << h.second << "\r\n";
        }

        ss << "part data : \r\n";
        ss << mp.GetData() <<  "\r\n";
    }

    ss << "======FORMDATA======\r\n";

    for (auto const& fd : this->_formdata)
    {
        ss << "name => " << fd.name << "\r\n"
           << "is file => " << fd.IsFile() << "\r\n"
           << "data => \r\n" << fd.data << "\r\n";
    }

    return ss.str();
}

}//namespace da4qi4
