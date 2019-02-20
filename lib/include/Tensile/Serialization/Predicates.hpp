/*******************************************************************************
 *
 * MIT License
 *
 * Copyright (c) 2019 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#pragma once

#include <Tensile/Serialization/Base.hpp>

#include <Tensile/Predicates.hpp>
#include <Tensile/GEMMProblemPredicates.hpp>
#include <Tensile/AMDGPU.hpp>
#include <Tensile/AMDGPUPredicates.hpp>

namespace Tensile
{
    namespace Serialization
    {

        template <typename Object, typename IO>
        struct MappingTraits<std::shared_ptr<Predicates::Predicate<Object>>, IO>
        {
            using iot = IOTraits<IO>;

            static void mapping(IO & io, std::shared_ptr<Predicates::Predicate<Object>> & p)
            {
                std::string key;

                if(iot::outputting(io))
                    key = p->key();

                iot::mapRequired(io, "type", key);

                if(!SubclassMappingTraits<Predicates::Predicate<Object>, IO>::mapping(io, key, p))
                    iot::setError(io, "Unknown predicate type " + key);
            }
        };

        /**
         * This is not part of the main traits classes but it eliminates the duplication of the
         * "Base" predicate types for different object types.
         */
        template <typename Object, typename IO>
        struct PredicateMappingTraits:
            public DefaultSubclassMappingTraits<PredicateMappingTraits<Object, IO>,
                                                Predicates::Predicate<Object>,
                                                IO>
        {
            using Self = PredicateMappingTraits<Object, IO>;
            using Base = DefaultSubclassMappingTraits<Self, Predicates::Predicate<Object>, IO>;
            using SubclassMap = typename Base::SubclassMap;
            const static SubclassMap subclasses;

            static typename Base::SubclassMap GetSubclasses()
            {
                return SubclassMap(
                {
                    Base::template Pair<Predicates::True <Object>>(),
                    Base::template Pair<Predicates::False<Object>>(),
                    Base::template Pair<Predicates::And  <Object>>(),
                    Base::template Pair<Predicates::Or   <Object>>(),
                    Base::template Pair<Predicates::Not  <Object>>()
                });
            }
        };

        template <typename Object, typename IO>
        const typename PredicateMappingTraits<Object, IO>::SubclassMap
            PredicateMappingTraits<Object, IO>::subclasses =
        PredicateMappingTraits<Object, IO>::GetSubclasses();

        template <typename Object, typename IO>
        struct MappingTraits<Predicates::True<Object>, IO>:
        public EmptyMappingTraits<Predicates::True<Object>, IO> {};

        template <typename Object, typename IO>
        struct MappingTraits<Predicates::False<Object>, IO>:
        public EmptyMappingTraits<Predicates::False<Object>, IO> {};

        template <typename Object, typename IO>
        struct MappingTraits<Predicates::And<Object>, IO>:
        public ValueMappingTraits<Predicates::And<Object>, IO> {};

        template <typename Object, typename IO>
        struct MappingTraits<Predicates::Or<Object>, IO>:
        public ValueMappingTraits<Predicates::Or<Object>, IO> {};

        template <typename Object, typename IO>
        struct MappingTraits<Predicates::Not<Object>, IO>:
        public ValueMappingTraits<Predicates::Not<Object>, IO> {};


        template <typename IO>
        struct SubclassMappingTraits<Predicates::Predicate<GEMMProblem>, IO>:
            public DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<GEMMProblem>, IO>,
                                                Predicates::Predicate<GEMMProblem>,
                                                IO>
        {
            using Self = SubclassMappingTraits<Predicates::Predicate<GEMMProblem>, IO>;
            using Base = DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<GEMMProblem>, IO>,
                                                      Predicates::Predicate<GEMMProblem>,
                                                      IO>;
            using SubclassMap = typename Base::SubclassMap;
            const static SubclassMap subclasses;

            using Generic = PredicateMappingTraits<GEMMProblem, IO>;

            static SubclassMap GetSubclasses()
            {
                SubclassMap rv(
                {
                    Base::template Pair<Predicates::GEMM::ADimensionOrder>(),
                    Base::template Pair<Predicates::GEMM::BDimensionOrder>(),
                    Base::template Pair<Predicates::GEMM::CDimensionOrder>(),
                    Base::template Pair<Predicates::GEMM::DDimensionOrder>(),
                    Base::template Pair<Predicates::GEMM::IDivisible     >(),
                    Base::template Pair<Predicates::GEMM::JDivisible     >(),
                    Base::template Pair<Predicates::GEMM::KDivisible     >(),
                    Base::template Pair<Predicates::GEMM::LDivisible     >(),
                    Base::template Pair<Predicates::GEMM::CDStridesEqual >(),
                    Base::template Pair<Predicates::GEMM::LDCEqualsLDD   >(),
                    Base::template Pair<Predicates::GEMM::UseBeta        >()
                });

                auto gmap = Generic::GetSubclasses();
                rv.insert(gmap.begin(), gmap.end());

                return rv;
            }

        };

        template <typename IO>
        using GEMMProblemPredicateSMT = SubclassMappingTraits<Predicates::Predicate<GEMMProblem>, IO>;

        template <typename IO>
        const typename GEMMProblemPredicateSMT<IO>::SubclassMap
        GEMMProblemPredicateSMT<IO>::subclasses = GEMMProblemPredicateSMT<IO>::GetSubclasses();

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::ADimensionOrder, IO>:
        public ValueMappingTraits<Predicates::GEMM::ADimensionOrder, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::BDimensionOrder, IO>:
        public ValueMappingTraits<Predicates::GEMM::BDimensionOrder, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::CDimensionOrder, IO>:
        public ValueMappingTraits<Predicates::GEMM::CDimensionOrder, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::DDimensionOrder, IO>:
        public ValueMappingTraits<Predicates::GEMM::DDimensionOrder, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::IDivisible, IO>:
        public ValueMappingTraits<Predicates::GEMM::IDivisible, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::JDivisible, IO>:
        public ValueMappingTraits<Predicates::GEMM::JDivisible, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::KDivisible, IO>:
        public ValueMappingTraits<Predicates::GEMM::KDivisible, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::LDivisible, IO>:
        public ValueMappingTraits<Predicates::GEMM::LDivisible, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::CDStridesEqual, IO>:
        public EmptyMappingTraits<Predicates::GEMM::CDStridesEqual, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::LDCEqualsLDD, IO>:
        public EmptyMappingTraits<Predicates::GEMM::LDCEqualsLDD, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GEMM::UseBeta, IO>:
        public ValueMappingTraits<Predicates::GEMM::UseBeta, IO> {};

        template <typename IO>
        struct SubclassMappingTraits<Predicates::Predicate<Hardware>, IO>:
            public DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<Hardware>, IO>,
                                                Predicates::Predicate<Hardware>,
                                                IO>
        {
            using Self = SubclassMappingTraits<Predicates::Predicate<Hardware>, IO>;
            using Base = DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<Hardware>, IO>,
                                                      Predicates::Predicate<Hardware>,
                                                      IO>;
            using SubclassMap = typename Base::SubclassMap;
            const static SubclassMap subclasses;

            using Generic = PredicateMappingTraits<Hardware, IO>;

            static SubclassMap GetSubclasses()
            {
                SubclassMap rv(
                {
                    Base::template Pair<Predicates::IsSubclass<Hardware, AMDGPU>>()
                });

                auto gmap = Generic::GetSubclasses();
                rv.insert(gmap.begin(), gmap.end());

                return rv;
            }
        };

        template <typename IO>
        using HardwarePredicateSMT = SubclassMappingTraits<Predicates::Predicate<Hardware>, IO>;

        template <typename IO>
        const typename HardwarePredicateSMT<IO>::SubclassMap
            HardwarePredicateSMT<IO>::subclasses =
                HardwarePredicateSMT<IO>::GetSubclasses();

        template <typename IO>
        struct MappingTraits<Predicates::IsSubclass<Hardware, AMDGPU>, IO>:
        public ValueMappingTraits<Predicates::IsSubclass<Hardware, AMDGPU>, IO> {};

        template <typename IO>
        struct SubclassMappingTraits<Predicates::Predicate<AMDGPU>, IO>:
            public DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<AMDGPU>, IO>,
                                                Predicates::Predicate<AMDGPU>,
                                                IO>
        {
            using Self = SubclassMappingTraits<Predicates::Predicate<AMDGPU>, IO>;
            using Base = DefaultSubclassMappingTraits<SubclassMappingTraits<Predicates::Predicate<AMDGPU>, IO>,
                                                      Predicates::Predicate<AMDGPU>,
                                                      IO>;
            using SubclassMap = typename Base::SubclassMap;
            const static SubclassMap subclasses;

            using Generic = PredicateMappingTraits<AMDGPU, IO>;

            static SubclassMap GetSubclasses()
            {
                SubclassMap rv(
                {
                    Base::template Pair<Predicates::GPU::ProcessorEqual>(),
                    Base::template Pair<Predicates::GPU::RunsKernelTargeting>()
                });

                auto gmap = Generic::GetSubclasses();
                rv.insert(gmap.begin(), gmap.end());

                return rv;
            }
        };

        template <typename IO>
        using AMDGPUPredicateSMT = SubclassMappingTraits<Predicates::Predicate<AMDGPU>, IO>;

        template <typename IO>
        const typename AMDGPUPredicateSMT<IO>::SubclassMap
            AMDGPUPredicateSMT<IO>::subclasses =
                AMDGPUPredicateSMT<IO>::GetSubclasses();

        template <typename IO>
        struct MappingTraits<Predicates::GPU::ProcessorEqual, IO>:
        public ValueMappingTraits<Predicates::GPU::ProcessorEqual, IO> {};

        template <typename IO>
        struct MappingTraits<Predicates::GPU::RunsKernelTargeting, IO>:
        public ValueMappingTraits<Predicates::GPU::RunsKernelTargeting, IO> {};

        template <typename IO>
        struct EnumTraits<AMDGPU::Processor, IO>
        {
            using iot = IOTraits<IO>;

            static void enumeration(IO & io, AMDGPU::Processor & value)
            {
                iot::enumCase(io, value, "gfx803", AMDGPU::Processor::gfx803);
                iot::enumCase(io, value, "gfx900", AMDGPU::Processor::gfx900);
                iot::enumCase(io, value, "gfx906", AMDGPU::Processor::gfx906);
            }
        };
    }
}
