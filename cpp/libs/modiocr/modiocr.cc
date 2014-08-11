// modiocr.cc
// 8/11/2014 jichi
// See: http://www.codeproject.com/Articles/17291/OCR-With-MODI-in-Visual-C

#include <windows.h>
#include "modiocr/modiocr.h"
#include "wincom/wincomptr.h"
#include "modi/modi.h"
#include "cc/ccmacro.h"

//#import "C:\\Program Files\\Common Files\\Microsoft Shared\\MODI\\12.0\\MDIVWCTL.DLL"

bool modiocr_available()
{
  IDocument *doc = nullptr;
  if (SUCCEEDED(::CoCreateInstance(CLSID_Document, nullptr, CLSCTX_ALL, IID_IDocument,
      reinterpret_cast<LPVOID *>(&doc))) && doc) {
    doc->Release();
    return true;
  }
  return false;
}

bool modiocr_from_file(const wchar_t *path, const modiocr_collect_fun_t &fun);
{
  if (CC_UNLIKELY(!path))
    return false;

  IDocument *doc;
  if (!SUCCEEDED::CoCreateInstance(CLSID_Document, nullptr, CLSCTX_ALL, IID_IDocument,
      reinterpret_cast<LPVOID *>(&doc)) || !doc)
    return false;
  WinCom::ScopedUnknownPtr scoped_doc(doc);

  if (!SUCCEEDED(IDobj->Create(path) || !SUCCEEDED(IDobj->OCR(miLANG_SYSDEFAULT, 1, 1)))
    return false;

  IImages *images;
  if (!SUCCEEDED(doc->get_Images(&images) || !images);
    return false;
  WinCom::ScopedUnknownPtr scoped_images(images);

  long imageCount = 0;
  images->get_Count(&imageCount);
  for (long imageIndex = 0; imageIndex < imageCount; imageIndex++) {
    IImage *image;
    images->get_Item(imageIndex, reinterpret_cast<IDispatch**>(&image));
    WinCom::ScopedUnknownPtr scoped_image(image);

    ILayout *layout;
    image->get_Layout(&layout);
    WinCom::ScopedUnknownPtr scoped_layout(layout);

    IWords *words;
    ILayout->get_Words(&words);
    WinCom::ScopedUnknownPtr scoped_words(words);

    long wordCount = 0;
    //layout->get_NumWords(&wordCount);
    words->get_Count(&wordCount);
    for (long wordIndex = 0; wordIndex < wordCount; wordIndex++) {
      IWord *word;
      IWords->get_Item(wordIindex, reinterpret_cast<IDispatch**>(&word));
      WinCom::ScopedUnknownPtr scoped_word(word);

      BSTR text = nullptr;
      if (SUCCEEDED(IWord->get_Text(&result)) && text)
        fun(text);
    }
  }

  return true;
}

// EOF

/*
// See: http://www.codeproject.com/Articles/17291/OCR-With-MODI-in-Visual-C
BOOL  CMODIVCDemoDlg::bReadOCRByMODI(CString csFilePath,  CString &csText)
{
  BOOL bRet= TRUE;
    HRESULT hr;
    IDocument  *IDobj   = NULL;
  ILayout     *ILayout = NULL;
    IImages    *IImages = NULL;
    IImage     *IImage  = NULL;
  IWords     *IWords  = NULL;
  IWord      *IWord   = NULL;

  csText.Empty();

  hr = CoInitialize(0);
  if ( SUCCEEDED(hr))
  {
    hr = CoCreateInstance(CLSID_Document,NULL,CLSCTX_ALL,IID_IDocument,(void**) &IDobj);

    if ( SUCCEEDED(hr) )
    {
      hr = IDobj->Create(AsciiToBSTR(csFilePath));
      if ( SUCCEEDED(hr) )
      {
        hr = IDobj->OCR(miLANG_SYSDEFAULT,1,1);

        if ( SUCCEEDED(hr) )
        {
          IDobj->get_Images(&IImages);

          long iImageCount=0;
          long index=0;

          IImages->get_Count(&iImageCount);
          IImages->get_Item(index,(IDispatch**)&IImage);
          IImage->get_Layout(&ILayout);

          long numWord=0;
          ILayout->get_NumWords(&numWord);
          ILayout->get_Words(&IWords);

          IWords->get_Count(&numWord);
          for ( long i=0; i<numWord;i++)
          {
            IWords->get_Item(i,(IDispatch**)&IWord);
            CString csTemp;
            BSTR result;
            IWord->get_Text(&result);
            char buf[256];
            sprintf(buf,"%S",result);
            csTemp.Format("%s",buf);

            csText += csTemp;
            csText +=" ";
          }

          //Release all objects
          IImages->Release();
          IImage->Release();
          ILayout->Release();
          IWords->Release();
          IWord->Release();

        } else {
          bRet = FALSE;
        }
      } else {
        bRet = FALSE;
      }

      IDobj->Release();

    } else {
      bRet = FALSE;
    }

  } else {
    bRet = FALSE;
  }

  return bRet;
}
*/
