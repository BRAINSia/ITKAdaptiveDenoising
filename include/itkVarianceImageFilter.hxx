/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkVarianceImageFilter_hxx
#define itkVarianceImageFilter_hxx

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

namespace itk
{
template <typename TInputImage, typename TOutputImage>
VarianceImageFilter<TInputImage, TOutputImage>::VarianceImageFilter()
{
  this->DynamicMultiThreadingOff();
}

template <typename TInputImage, typename TOutputImage>
void
VarianceImageFilter<TInputImage, TOutputImage>::ThreadedGenerateData(
  const OutputImageRegionType & outputRegionForThread,
  ThreadIdType                  threadId)
{
  unsigned int i;

  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;

  ConstNeighborhoodIterator<InputImageType> bit;
  ImageRegionIterator<OutputImageType>      it;

  // Allocate output
  typename OutputImageType::Pointer     output = this->GetOutput();
  typename InputImageType::ConstPointer input = this->GetInput();

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>                        bC;
  faceList = bC(input, outputRegionForThread, this->GetRadius());

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  // support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  InputRealType sum;
  InputRealType sumOfSquares;

  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (fit = faceList.begin(); fit != faceList.end(); ++fit)
  {
    bit = ConstNeighborhoodIterator<InputImageType>(this->GetRadius(), input, *fit);
    unsigned int neighborhoodSize = bit.Size();
    it = ImageRegionIterator<OutputImageType>(output, *fit);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    while (!bit.IsAtEnd())
    {
      sum = NumericTraits<InputRealType>::ZeroValue();
      sumOfSquares = NumericTraits<InputRealType>::ZeroValue();

      for (i = 0; i < neighborhoodSize; ++i)
      {
        sum += static_cast<InputRealType>(bit.GetPixel(i));
        sumOfSquares += itk::Math::sqr(static_cast<InputRealType>(bit.GetPixel(i)));
      }

      // get the variance value
      const double    num = static_cast<double>(neighborhoodSize);
      OutputPixelType var = (sumOfSquares - (itk::Math::sqr(sum) / num)) / (num - 1.0);

      it.Set(var);

      ++bit;
      ++it;
      progress.CompletedPixel();
    }
  }
}
} // end namespace itk

#endif
