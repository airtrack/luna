#ifndef GC_OBJECT_H
#define GC_OBJECT_H

#include <functional>
#include <deque>
#include <fstream>

namespace luna
{
    // Generations of GC object
    enum GCGeneration
    {
        GCGen0,         // Youngest generation
        GCGen1,         // Mesozoic generation
        GCGen2,         // Oldest generation
    };

    // GC flag for mark GC object
    enum GCFlag
    {
        GCFlag_White,
        GCFlag_Black,
    };

    // GC object type allocated by GC
    enum GCObjectType
    {
        GCObjectType_Table = 1,
        GCObjectType_Function,
        GCObjectType_Closure,
        GCObjectType_Upvalue,
        GCObjectType_String,
    };

    class Table;
    class Function;
    class Closure;
    class Upvalue;
    class String;

    // Visitor for visit all GC objects
    class GCObjectVisitor
    {
    public:
        // Need visit all GC object members when return true
        virtual bool Visit(Table *) = 0;
        virtual bool Visit(Function *) = 0;
        virtual bool Visit(Closure *) = 0;
        virtual bool Visit(Upvalue *) = 0;
        virtual bool Visit(String *) = 0;
    };

    // Base class of GC objects, GC use this class to manipulate
    // all GC objects
    class GCObject
    {
        friend class GC;
        friend class MinorMarkVisitor;
        friend class BarrieredMarkVisitor;
        friend class MajorMarkVisitor;
        friend bool CheckBarrier(GCObject *);
    public:
        GCObject();
        virtual ~GCObject() = 0;

        virtual void Accept(GCObjectVisitor *) = 0;

    private:
        // Pointing next GCObject in current generation
        GCObject *next_;
        // Generation flag
        unsigned int generation_ : 2;
        // GC flag
        unsigned int gc_ : 2;
        // GC object type
        unsigned int gc_obj_type_ : 4;
    };

    // GC object barrier checker
    inline bool CheckBarrier(GCObject *obj) { return obj->generation_ != GCGen0; }
    #define CHECK_BARRIER(gc, obj) if (luna::CheckBarrier(obj)) gc.SetBarrier(obj)

    class GC
    {
    public:
        typedef std::function<void (GCObjectVisitor *)> RootTravelType;
        typedef std::function<void (GCObject *, unsigned int)> GCObjectDeleter;

        struct DefaultDeleter
        {
            inline void operator () (GCObject *obj, unsigned int) const
            { delete obj; }
        };

        explicit GC(const GCObjectDeleter &obj_deleter = DefaultDeleter(), bool log = false);
        ~GC();

        void ResetDeleter(const GCObjectDeleter &obj_deleter = DefaultDeleter())
        { obj_deleter_ = obj_deleter; }

        // Set minor and major root travel functions
        void SetRootTraveller(const RootTravelType &minor, const RootTravelType &major);

        // Alloc GC objects
        Table * NewTable(GCGeneration gen = GCGen0);
        Function * NewFunction(GCGeneration gen = GCGen2);
        Closure * NewClosure(GCGeneration gen = GCGen0);
        Upvalue * NewUpvalue(GCGeneration gen = GCGen0);
        String * NewString(GCGeneration gen = GCGen0);

        // Set GC object barrier
        void SetBarrier(GCObject *obj);

        // Check run GC
        void CheckGC();

    private:
        struct GenInfo
        {
            // Pointing to GC object list
            GCObject *gen_;
            // Count of GC objects
            unsigned int count_;
            // Current threshold count of GC objects
            unsigned int threshold_count_;

            GenInfo() : gen_(nullptr), count_(0), threshold_count_(0) { }
        };

        void SetObjectGen(GCObject *obj, GCGeneration gen);

        // Run minor and major GC
        void MinorGC();
        void MajorGC();

        void MinorGCMark();
        void MinorGCSweep();

        void MajorGCMark();
        void MajorGCSweep();

        void SweepGeneration(GenInfo &gen);

        // Adjust GenInfo's threshold_count_ by alived_count
        void AdjustThreshold(unsigned int alived_count, GenInfo &gen,
                             unsigned int min_threshold,
                             unsigned int max_threshold);

        // Delete generation all objects
        void DestroyGeneration(GenInfo &gen);

        static const unsigned int kGen0InitThresholdCount = 512;
        static const unsigned int kGen1InitThresholdCount = 512;
        static const unsigned int kGen0MaxThresholdCount = 2048;
        static const unsigned int kGen1MaxThresholdCount = 102400;

        // Youngest generation
        GenInfo gen0_;
        // Mesozoic generation
        GenInfo gen1_;
        // Oldest generation
        GenInfo gen2_;

        // Minor root traveller
        RootTravelType minor_traveller_;
        // Major root traveller
        RootTravelType major_traveller_;

        // Barriered GC objects
        std::deque<GCObject *> barriered_;

        // GC object Deleter
        GCObjectDeleter obj_deleter_;
        // Log file
        std::ofstream log_stream_;
    };
} // namespace luna

#endif // GC_OBJECT_H
