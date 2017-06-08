#pragma once

namespace xzero 
{
    class KG_UnCopyable
    {
    protected:
        KG_UnCopyable() {}                                              // constructor
        ~KG_UnCopyable() {}                                             // destructor
    private:
        KG_UnCopyable(const KG_UnCopyable &) {}                         // copy constructor
        KG_UnCopyable &operator=(const KG_UnCopyable &) {}              // copy assignment
    };

    class KG_UnConstructable
    {
    protected:
        ~KG_UnConstructable() {}                                        // destructor
    private:
        KG_UnConstructable() {}                                         // constructor
        KG_UnConstructable(const KG_UnConstructable &) {}               // copy constructor
        KG_UnConstructable &operator=(const KG_UnConstructable &) {}    // copy assignment
    };
}