/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#ifdef JAVA

#include "onload.h"

JavaVM* m_javaVM = 0;


extern "C"
{

    //------------------------------------------------------------------------
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* aVm, void* aReserved)
    {
        // cache java VM
        m_javaVM = aVm;

        return JNI_VERSION_1_6;
    }

} // extern "C"


JavaVM* get_imebra_javaVM()
{
    return m_javaVM;
}

#endif
