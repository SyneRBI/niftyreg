/** @file _reg_optimiser.cpp
 * @author Marc Modat
 * @date 20/07/2012
 */

#ifndef _REG_OPTIMISER_CPP
#define _REG_OPTIMISER_CPP

#include "_reg_optimiser.h"

/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_optimiser<T>::reg_optimiser()
{
    this->dofNumber=0;
    this->dofNumber_b=0;
    this->ndim=3;
    this->optimiseX=true;
    this->optimiseY=true;
    this->optimiseZ=true;
    this->currentDOF=NULL;
    this->currentDOF_b=NULL;
    this->bestDOF=NULL;
    this->bestDOF_b=NULL;
    this->backward=false;

#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_optimiser<T>::reg_optimiser() called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_optimiser<T>::~reg_optimiser()
{
    if(this->bestDOF!=NULL)
        free(this->bestDOF);
    this->bestDOF=NULL;
    if(this->bestDOF_b!=NULL)
        free(this->bestDOF_b);
    this->bestDOF_b=NULL;
#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_optimiser<T>::~reg_optimiser() called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::Initialise(size_t nvox,
                                  int dim,
                                  bool optX,
                                  bool optY,
                                  bool optZ,
                                  size_t maxit,
                                  size_t start,
                                  InterfaceOptimiser *obj,
                                  T *cppData,
                                  T *gradData,
                                  size_t nvox_b,
                                  T *cppData_b,
                                  T *gradData_b
                                  )
{
    this->dofNumber=nvox;
    this->ndim=dim;
    this->optimiseX=optX;
    this->optimiseY=optY;
    this->optimiseZ=optZ;
    this->maxIterationNumber=maxit;
    this->currentIterationNumber=start;
    this->currentDOF=cppData;
    if(this->bestDOF!=NULL) free(this->bestDOF);
    this->bestDOF=(T *)malloc(this->dofNumber*sizeof(T));
    memcpy(this->bestDOF,this->currentDOF,this->dofNumber*sizeof(T));
    if( gradData!=NULL)
        this->gradient=gradData;


    if(nvox_b>0) this->dofNumber_b=nvox_b;
    if(cppData_b!=NULL){
        this->currentDOF_b=cppData_b;
        this->backward=true;
        if(this->bestDOF_b!=NULL) free(this->bestDOF_b);
        this->bestDOF_b=(T *)malloc(this->dofNumber*sizeof(T));
        memcpy(this->bestDOF_b,this->currentDOF_b,this->dofNumber_b*sizeof(T));
    }
    if(gradData_b!=NULL)
        this->gradient_b=gradData_b;

    this->objFunc=obj;
    this->bestObjFunctionValue = this->currentObjFunctionValue =
            this->objFunc->GetObjectiveFunctionValue();

#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_optimiser<T>::Initialise called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::RestoreBestDOF()
{
    // restore forward transformation
    memcpy(this->currentDOF,this->bestDOF,this->dofNumber*sizeof(T));
    // restore backward transformation if required
    if(this->currentDOF_b!=NULL && this->bestDOF_b!=NULL && this->dofNumber_b>0)
        memcpy(this->currentDOF_b,this->bestDOF_b,this->dofNumber_b*sizeof(T));
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::StoreCurrentDOF()
{
    // save forward transformation
    memcpy(this->bestDOF,this->currentDOF,this->dofNumber*sizeof(T));
    // save backward transformation if required
    if(this->currentDOF_b!=NULL && this->bestDOF_b!=NULL && this->dofNumber_b>0)
        memcpy(this->bestDOF_b,this->currentDOF_b,this->dofNumber_b*sizeof(T));
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::Perturbation(float length)
{
    // initialise the randomiser
    srand(time(NULL));
    // Reset the number of iteration
    this->currentIterationNumber=0;
    // Create some perturbation for degree of freedom
    for(size_t i=0;i<this->dofNumber;++i){
        this->currentDOF[i]=this->bestDOF[i] + length * (float)(rand() - RAND_MAX/2) / ((float)RAND_MAX/2.0f);
    }
    if(this->backward==true){
        for(size_t i=0;i<this->dofNumber_b;++i){
            this->currentDOF_b[i]=this->bestDOF_b[i] + length * (float)(rand() % 2001 - 1000) / 1000.f;
        }
    }
    this->StoreCurrentDOF();
    this->currentObjFunctionValue=this->bestObjFunctionValue=
            this->objFunc->GetObjectiveFunctionValue();
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::NormaliseGradient()
{
    // First compute the gradient max length for normalisation purpose
    T maxGradValue=0;
    size_t voxNumber = this->GetVoxNumber();
    size_t voxNumber_b = this->GetVoxNumber_b();
    T *ptrX = this->gradient;
    T *ptrY = &ptrX[voxNumber];
    if(this->ndim>2){
        T *ptrZ = &ptrY[voxNumber];
        for(int i=0; i<voxNumber; i++){
            T valX=0,valY=0,valZ=0;
            if(this->GetOptimiseX()==true)
                valX = *ptrX++;
            if(this->GetOptimiseY()==true)
                valY = *ptrY++;
            if(this->GetOptimiseZ()==true)
                valZ = *ptrZ++;
            float length = (float)(sqrt(valX*valX + valY*valY + valZ*valZ));
            maxGradValue = (length>maxGradValue)?length:maxGradValue;
        }
    }
    else{
        for(int i=0; i<voxNumber; i++){
            T valX=0,valY=0;
            if(this->GetOptimiseX()==true)
                valX = *ptrX++;
            if(this->GetOptimiseY()==true)
                valY = *ptrY++;
            float length = (float)(sqrt(valX*valX + valY*valY));
            maxGradValue = (length>maxGradValue)?length:maxGradValue;
        }
    }
    if(this->backward){
        T maxGradValue_b=0;
        T *ptrX_b = this->gradient_b;
        T *ptrY_b = &ptrX[voxNumber_b];
        if(this->ndim>2){
            T *ptrZ_b = &ptrY_b[voxNumber_b];
            for(int i=0; i<voxNumber_b; i++){
                T valX=0,valY=0,valZ=0;
                if(this->optimiseX==true)
                    valX = *ptrX_b++;
                if(this->optimiseY==true)
                    valY = *ptrY_b++;
                if(this->optimiseZ==true)
                    valZ = *ptrZ_b++;
                float length = (float)(sqrt(valX*valX + valY*valY + valZ*valZ));
                maxGradValue_b = (length>maxGradValue_b)?length:maxGradValue_b;
            }
        }
        else{
            for(int i=0; i<voxNumber_b; i++){
                T valX=0,valY=0;
                if(this->optimiseX==true)
                    valX = *ptrX_b++;
                if(this->optimiseY==true)
                    valY = *ptrY_b++;
                float length = (float)(sqrt(valX*valX + valY*valY));
                maxGradValue_b = (length>maxGradValue_b)?length:maxGradValue_b;
            }
        }
        maxGradValue = maxGradValue + maxGradValue_b;
    }
#ifndef NDEBUG
    printf("[NiftyReg DEBUG] Objective function gradient maximal length: %g\n",maxGradValue);
#endif

    for(size_t i=0;i<this->dofNumber;++i){
        this->gradient[i] = this->gradient[i] / maxGradValue;
    }
    if(this->backward){
        for(size_t i=0;i<this->dofNumber_b;++i){
            this->gradient_b[i] = this->gradient_b[i] / maxGradValue;
        }
    }
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_optimiser<T>::Optimise(float maxLength,
                                float smallLength,
                                float &startLength)
{
    size_t lineIteration=0;
    float addedLength=0;
    float currentLength=startLength;

    this->NormaliseGradient();

    // Start performing the line search
    while(currentLength>smallLength &&
          lineIteration<12 &&
          this->currentIterationNumber<this->maxIterationNumber){

        // Compute the gradient normalisation value
        float normValue = -currentLength;

        this->objFunc->UpdateParameters(normValue);

        // Compute the new value
        this->currentObjFunctionValue=this->objFunc->GetObjectiveFunctionValue();

        // Check if the update lead to an improvement of the objective function
        if(this->currentObjFunctionValue > this->bestObjFunctionValue){
#ifndef NDEBUG
            printf("[NiftyReg DEBUG] [%i] objective function: %g | Increment %g | ACCEPTED\n",
                   (int)this->currentIterationNumber,
                   this->currentObjFunctionValue,
                   currentLength);
#endif
            // Improvement - Save the new objective function value
            this->bestObjFunctionValue=this->currentObjFunctionValue;
            // Update the total added length
            addedLength += currentLength;
            // Increase the step size
            currentLength *= 1.1f;
            currentLength = (currentLength<maxLength)?currentLength:maxLength;
            // Save the current deformation parametrisation
            this->StoreCurrentDOF();
        }
        else{
#ifndef NDEBUG
            printf("[NiftyReg DEBUG] [%i] objective function: %g | Increment %g | REJECTED\n",
                   (int)this->currentIterationNumber,
                   this->currentObjFunctionValue,
                   currentLength);
#endif
            // No improvement - Decrease the step size
            currentLength*=0.5;
        }
        this->IncrementCurrentIterationNumber();
        ++lineIteration;
    }
    // update the current size for the next iteration
    startLength=addedLength;
    // Restore the last best deformation parametrisation
    this->RestoreBestDOF();
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_conjugateGradient<T>::reg_conjugateGradient()
    :reg_optimiser<T>::reg_optimiser()
{
    this->array1=NULL;
    this->array2=NULL;
    this->array1_b=NULL;
    this->array2_b=NULL;

#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_conjugateGradient<T>::reg_conjugateGradient() called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_conjugateGradient<T>::~reg_conjugateGradient()
{
    if(this->array1!=NULL)
        free(this->array1);
    this->array1=NULL;

    if(this->array2!=NULL)
        free(this->array2);
    this->array2=NULL;

    if(this->array1_b!=NULL)
        free(this->array1_b);
    this->array1_b=NULL;

    if(this->array2_b!=NULL)
        free(this->array2_b);
    this->array2_b=NULL;

#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_conjugateGradient<T>::~reg_conjugateGradient() called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_conjugateGradient<T>::Initialise(size_t nvox,
                                          int dim,
                                          bool optX,
                                          bool optY,
                                          bool optZ,
                                          size_t maxit,
                                          size_t start,
                                          InterfaceOptimiser *o,
                                          T *cppData,
                                          T *gradData,
                                          size_t nvox_b,
                                          T *cppData_b,
                                          T *gradData_b
                                          )
{
    reg_optimiser<T>::Initialise(nvox,
                                 dim,
                                 optX,
                                 optY,
                                 optZ,
                                 maxit,
                                 start,
                                 o,
                                 cppData,
                                 gradData,
                                 nvox_b,
                                 cppData_b,
                                 gradData_b
                                 );
    this->firstcall=true;
    this->array1=(T *)malloc(this->dofNumber*sizeof(T));
    this->array2=(T *)malloc(this->dofNumber*sizeof(T));

    if(cppData_b!=NULL && gradData_b!=NULL && nvox_b>0){
        this->array1_b=(T *)malloc(this->dofNumber_b*sizeof(T));
        this->array2_b=(T *)malloc(this->dofNumber_b*sizeof(T));
    }

#ifndef NDEBUG
    printf("[NiftyReg DEBUG] reg_conjugateGradient<T>::Initialise called\n");
#endif
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_conjugateGradient<T>::UpdateGradientValues()
{
#if defined (_WIN32)
    int i;
#else
    size_t i;
#endif
    size_t num=this->dofNumber;
    size_t num_b=this->dofNumber_b;
    T *gradientPtr = this->gradient;
    T *array1Ptr = this->array1;
    T *array2Ptr = this->array2;

    T *gradientPtr_b = this->gradient_b;
    T *array1Ptr_b = this->array1_b;
    T *array2Ptr_b = this->array2_b;

    if(this->firstcall==true){
#ifndef NDEBUG
        printf("[NiftyReg DEBUG] Conjugate gradient initialisation\n");
#endif
        // first conjugate gradient iteration
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num,array1Ptr,array2Ptr,gradientPtr) \
    private(i)
#endif
        for(i=0; i<num;i++){
            array2Ptr[i] = array1Ptr[i] = - gradientPtr[i];
        }
        if(this->dofNumber_b>0){
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num_b,array1Ptr_b,array2Ptr_b,gradientPtr_b) \
    private(i)
#endif
            for(i=0; i<num_b;i++){
                array2Ptr_b[i] = array1Ptr_b[i] = - gradientPtr_b[i];
            }
        }
        this->firstcall=false;
    }
    else{
#ifndef NDEBUG
        printf("[NiftyReg DEBUG] Conjugate gradient update\n");
#endif
        double dgg=0.0, gg=0.0;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num,array1Ptr,array2Ptr,gradientPtr) \
    private(i) \
    reduction(+:gg) \
    reduction(+:dgg)
#endif
        for(i=0; i<num;i++){
            gg += array2Ptr[i] * array1Ptr[i];
            dgg += (gradientPtr[i] + array1Ptr[i]) * gradientPtr[i];
        }
        double gam = dgg/gg;

        if(this->dofNumber_b>0){
            double dgg_b=0.0, gg_b=0.0;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num_b,array1Ptr_b,array2Ptr_b,gradientPtr_b) \
    private(i) \
    reduction(+:gg_b) \
    reduction(+:dgg_b)
#endif
            for(i=0; i<num_b;i++){
                gg_b += array2Ptr_b[i] * array1Ptr_b[i];
                dgg_b += (gradientPtr_b[i] + array1Ptr_b[i]) * gradientPtr_b[i];
            }
            gam = (dgg+dgg_b)/(gg+gg_b);
        }
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num,array1Ptr,array2Ptr,gradientPtr,gam) \
    private(i)
#endif
        for(i=0; i<num;i++){
            array1Ptr[i] = - gradientPtr[i];
            array2Ptr[i] = (array1Ptr[i] + gam * array2Ptr[i]);
            gradientPtr[i] = - array2Ptr[i];
        }
        if(this->dofNumber_b>0){
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(num_b,array1Ptr_b,array2Ptr_b,gradientPtr_b,gam) \
    private(i)
#endif
            for(i=0; i<num_b;i++){
                array1Ptr_b[i] = - gradientPtr_b[i];
                array2Ptr_b[i] = (array1Ptr_b[i] + gam * array2Ptr_b[i]);
                gradientPtr_b[i] = - array2Ptr_b[i];
            }
        }
    }
    return;
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_conjugateGradient<T>::Optimise(float maxLength,
                                        float smallLength,
                                        float &startLength)
{
    this->UpdateGradientValues();
    reg_optimiser<T>::Optimise(maxLength,
                               smallLength,
                               startLength);
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_conjugateGradient<T>::Perturbation(float length)
{
    reg_optimiser<T>::Perturbation(length);
    this->firstcall=true;
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_lbfgs<T>::reg_lbfgs()
    :reg_optimiser<T>::reg_optimiser()
{
    this->stepToKeep=5;
    this->oldDOF=NULL;
    this->oldGrad=NULL;
    this->diffDOF=NULL;
    this->diffGrad=NULL;
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
reg_lbfgs<T>::~reg_lbfgs()
{
    if(this->oldDOF!=NULL)
        free(this->oldDOF);
    this->oldDOF=NULL;
    if(this->oldGrad!=NULL)
        free(this->oldGrad);
    this->oldGrad=NULL;
    for(size_t i=0;i<this->stepToKeep;++i){
        if(this->diffDOF[i]!=NULL)
            free(this->diffDOF[i]);
        this->diffDOF[i]=NULL;
        if(this->diffGrad[i]!=NULL)
            free(this->diffGrad[i]);
        this->diffGrad[i]=NULL;
    }
    if(this->diffDOF!=NULL)
        free(this->diffDOF);
    this->diffDOF=NULL;
    if(this->diffGrad!=NULL)
        free(this->diffGrad);
    this->diffGrad=NULL;
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_lbfgs<T>::Initialise(size_t nvox,
                           int dim,
                           bool optX,
                           bool optY,
                           bool optZ,
                           size_t maxit,
                           size_t start,
                           InterfaceOptimiser *o,
                           T *cppData,
                           T *gradData,
                           size_t nvox_b,
                           T *cppData_b,
                           T *gradData_b)
{
    reg_optimiser<T>::Initialise(nvox,
                                 dim,
                                 optX,
                                 optY,
                                 optZ,
                                 maxit,
                                 start,
                                 o,
                                 cppData,
                                 gradData,
                                 nvox_b,
                                 cppData_b,
                                 gradData_b);
    this->stepToKeep=6;
    this->diffDOF=(T **)malloc(this->stepToKeep*sizeof(T *));
    this->diffGrad=(T **)malloc(this->stepToKeep*sizeof(T *));
    for(size_t i=0;i<this->stepToKeep;++i){
        this->diffDOF[i]=(T *)malloc(this->dofNumber*sizeof(T));
        this->diffGrad[i]=(T *)malloc(this->dofNumber*sizeof(T));
        if(this->diffDOF[i]==NULL || this->diffGrad[i]==NULL){
            fprintf(stderr, "[NiftyReg ERROR] reg_lbfgs running out of memory. Exit");
            exit(1);
        }
    }
    this->oldDOF=(T *)malloc(this->dofNumber*sizeof(T));
    this->oldGrad=(T *)malloc(this->dofNumber*sizeof(T));
    if(this->oldDOF==NULL || this->oldGrad==NULL){
        fprintf(stderr, "[NiftyReg ERROR] reg_lbfgs running out of memory. Exit");
        exit(1);
    }
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_lbfgs<T>::UpdateGradientValues()
{

}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
template <class T>
void reg_lbfgs<T>::Optimise(float maxLength,
                            float smallLength,
                            float &startLength)
{

    this->UpdateGradientValues();
    reg_optimiser<T>::Optimise(maxLength,
                               smallLength,
                               startLength);
}
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
/* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */
#endif // _REG_OPTIMISER_CPP