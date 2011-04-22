/******************************************************************************
#   Copyright 2006 Hermann Seib
#   Copyright 2010 Rapha�l Fran�ois
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "ceffect.h"
#include "cvsthost.h"
#include "../mainhost.h"
#include "vstbank.h"
#include "vstprogram.h"


using namespace vst;

/*===========================================================================*/
/* CEffect class members                                                     */
/*===========================================================================*/

/*****************************************************************************/
/* CEffect : constructor                                                     */
/*****************************************************************************/

CEffect::CEffect()
{
    pEffect = NULL;
    bEditOpen = false;
    bNeedIdle = false;
    bInEditIdle = false;
    bWantMidi = false;
    bInSetProgram = false;
    pMasterEffect = NULL;
    sName.clear();
}

/*****************************************************************************/
/* ~CEffect : destructor                                                     */
/*****************************************************************************/

CEffect::~CEffect()
{
    Unload();
}

/*****************************************************************************/
/* Load : loads the effect module                                            */
/*****************************************************************************/

bool CEffect::Load(MainHost *myHost, const QString &name)
{
    pluginLib = new QLibrary(name);

    if(!pluginLib->load()) {
        debug2(<< pluginLib->errorString() )
//        return false;
    }

    vstPluginFuncPtr entryPoint = (vstPluginFuncPtr)pluginLib->resolve("VSTPluginMain");
    if(!entryPoint)
        entryPoint = (vstPluginFuncPtr)pluginLib->resolve("main");
    if(!entryPoint) {
//        QMessageBox msgBox;
//        msgBox.setText(QObject::tr("error loading %1, no entry point").arg(name));
//        msgBox.exec();
        return false;
    }

    try
    {
        pEffect = entryPoint(&CVSTHost::AudioMasterCallback);// myHost->vstHost->AudioMasterCallback);
    }
    catch(...)
    {
//        QMessageBox msgBox;
//        msgBox.setText(QObject::tr("error loading %1, failed on entry point").arg(name));
//        msgBox.exec();
        pEffect = NULL;
    }

    if(!pEffect)
        return false;

    if (pEffect->magic != kEffectMagic) {
        pEffect = NULL;
//        QMessageBox msgBox;
//        msgBox.setText(QObject::tr("error loading %1, not a Vst plugin").arg(name));
//        msgBox.exec();
        return false;
    }
    sName = name;
    return true;
}

/*****************************************************************************/
/* Unload : unloads effect module                                            */
/*****************************************************************************/

bool CEffect::Unload()
{
    EffClose();
    pEffect = NULL;
    pluginLib->unload();
    pluginLib->deleteLater();
    return true;
}

/*****************************************************************************/
/* LoadBank : loads a .fxb file ... IF it's for this effect                  */
/*****************************************************************************/

bool CEffect::LoadBank(std::string *name)
{

    try
    {
        CFxBank bank(name);

        if (!bank.IsLoaded())
            throw (int)1;

        if (pEffect->uniqueID != bank.GetFxID()) {
            debug2(<<"CEffect::LoadBank ID doesn't match");
            throw (int)1;
          }

        EffBeginLoadBank(bank.GetPatchChunkInfo());

        if(bank.IsChunk()) {
            if(!(pEffect->flags & effFlagsProgramChunks)) {
                debug2(<< "CEffect::LoadBank chunk not handled")
                throw (int)1;
            }
            EffSetChunk(bank.GetChunk(),bank.GetChunkSize(),0);

        } else {

            //pEffect->numPrograms=bank.GetNumPrograms();

            for (int i = 0; i < bank.GetNumPrograms(); i++) {
                SFxProgram *prog = bank.GetProgram(i);

                VstPatchChunkInfo info;
                info = *bank.GetPatchChunkInfo();
                info.numElements=prog->numParams;
                EffBeginLoadProgram(&info);
                EffSetProgram(i);
                EffSetProgramName(prog->prgName);
                for(int j=0; j< prog->numParams; j++) {
                    float val = bank.GetProgParm(i,j);
                    EffSetParameter(j,val);
                }
            }
            EffSetProgram(0);

        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/*****************************************************************************/
/* SaveBank : saves current sound bank to a .fxb file                        */
/*****************************************************************************/

bool CEffect::SaveBank(std::string * name)
{
    CFxBank bank;

    if(pEffect->flags & effFlagsProgramChunks) {
        void *ptr=0;
        long size = EffGetChunk(&ptr,false);
        bank.SetSize(size);
        bank.SetChunk(ptr);
    } else {
        bank.SetSize(pEffect->numPrograms,pEffect->numParams);
        long oldPrg = EffGetProgram();
        for(int i=0; i<pEffect->numPrograms; i++) {
            char prgName[24];
            EffSetProgram(i);
            EffGetProgramName(prgName);
            bank.SetProgramName(i,prgName);

            for(int j=0; j<pEffect->numParams; j++) {
                bank.SetProgParm(i,j, EffGetParameter(j));
            }
        }
        EffSetProgram(oldPrg);
    }

    bank.SetFxID(pEffect->uniqueID);
    bank.SetFxVersion(pEffect->version);
    bank.SaveBank(name);

    return false;
}

bool CEffect::LoadProgram(std::string *name)
{
    try
    {
        CFxProgram progFile(name);

        if (!progFile.IsLoaded())
            throw (int)1;

        if (pEffect->uniqueID != progFile.GetFxID()) {
            debug2(<<"CEffect::LoadPreset ID doesn't match");
            throw (int)1;
        }

        if(progFile.IsChunk()) {
            if(!(pEffect->flags & effFlagsProgramChunks)) {
                debug2(<< "CEffect::LoadPreset chunk not handled")
                throw (int)1;
            }
            EffSetChunk(progFile.GetChunk(),progFile.GetChunkSize(),0);

        } else {
            SFxProgram *prog = progFile.GetProgram();

            VstPatchChunkInfo info;
            info = *progFile.GetPatchChunkInfo();
            info.numElements=prog->numParams;
            EffBeginLoadProgram(&info);
            EffSetProgramName(prog->prgName);
            for(int i=0; i< prog->numParams; i++) {
                float val = progFile.GetProgParm(i);
                EffSetParameter(i,val);
            }
        }

    }
    catch(...)
    {
        return false;
    }

    return true;
}

bool CEffect::SaveProgram(std::string *name)
{
    CFxProgram progFile;

    if(pEffect->flags & effFlagsProgramChunks) {
        void *ptr=0;
        long size = EffGetChunk(&ptr,false);
        progFile.SetChunkSize(size);
        progFile.SetChunk(ptr);
    } else {
        progFile.SetSize(pEffect->numParams);
        for(int i=0; i<pEffect->numParams; i++) {
            progFile.SetProgParm(i, EffGetParameter(i));
        }
    }

    char prgName[24];
    EffGetProgramName(prgName);
    progFile.SetProgramName(prgName);

    progFile.SetFxID(pEffect->uniqueID);
    progFile.SetFxVersion(pEffect->version);
    progFile.SaveProgram(name);

    return false;
}

/*****************************************************************************/
/* EffDispatch : calls an effect's dispatcher                                */
/*****************************************************************************/

long CEffect::EffDispatch(VstInt32 opCode, VstInt32 index, VstInt32 value, void *ptr, float opt)
{
    if (!pEffect)
        return 0;

    long disp=0L;
        disp = pEffect->dispatcher(pEffect, opCode, index, value, ptr, opt);

    return disp;
}

/*****************************************************************************/
/* EffProcess : calles an effect's process() function                        */
/*****************************************************************************/

void CEffect::EffProcess(float **inputs, float **outputs, long sampleframes)
{
    pEffect->process(pEffect, inputs, outputs, sampleframes);
}

/*****************************************************************************/
/* EffProcessReplacing : calls an effect's processReplacing() function       */
/*****************************************************************************/

void CEffect::EffProcessReplacing(float **inputs, float **outputs, long sampleframes)
{
    pEffect->processReplacing(pEffect, inputs, outputs, sampleframes);
 }

/*****************************************************************************/
/* EffProcessDoubleReplacing : calls an effect's processDoubleReplacing() f. */
/*****************************************************************************/

void CEffect::EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleFrames)
{
#if defined(VST_2_4_EXTENSIONS)
    pEffect->processDoubleReplacing(pEffect, inputs, outputs, sampleFrames);
#endif
}

/*****************************************************************************/
/* EffSetParameter : calls an effect's setParameter() function               */
/*****************************************************************************/

void CEffect::EffSetParameter(long index, float parameter)
{
        pEffect->setParameter(pEffect, index, parameter);
}

/*****************************************************************************/
/* EffGetParameter : calls an effect's getParameter() function               */
/*****************************************************************************/

float CEffect::EffGetParameter(long index)
{
    return pEffect->getParameter(pEffect, index);
}

VstIntPtr CEffect::OnMasterCallback(long opcode, long index, long value, void *ptr, float opt,long currentReturnCode)
{
    switch(opcode) {
        case audioMasterWantMidi : //6
            bWantMidi=true;
            return true;
    }

    return currentReturnCode;
}

QString CEffect::EffGetParamName(long index)
{
    char txt[32] ={0};
    EffDispatch(effGetParamName, index, 0, txt);
    return QString::fromAscii(txt);

}
