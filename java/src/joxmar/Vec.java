package joxmar;

// Basically C++'s vector class. Use Vec.Int for a vector of int, etc.
// TODO use codegen to make the .XXX classes, as they're basically all the same.

public final class Vec {
    public final static class Int {
        private final int START_CAPACITY = 8;
        private int[] _data = new int [START_CAPACITY];
        private int   _size = 0;  // num of currently stored elems

        public int size () {
            return _size;
        }
        public int get (int n) {
            Util.enforce (n < _size);
            return _data [n];
        }
        public void push (int num) {
            if (_size == _data.length) {
                reserve ((int) (size () * 1.5));
            }
            _data [_size] = num;
            ++_size;
        }

        // Copy out data to a new array
        public int[] toArray () {
            int[] res = new int [size()];
            System.arraycopy (_data, 0, res, 0, size());
            return res;
        }

        // Expand capacity to len
        public void reserve (int len) {
            Util.enforce (len >= size ());
            int[] newData = new int [len];
            System.arraycopy (_data, 0, newData, 0, _size);
            _data = newData;
        }
    }

    public final static class Double {
        private final int START_CAPACITY = 8;
        private double[] _data = new double [START_CAPACITY];
        private int      _size = 0;  // num of currently stored elems

        public int size () {
            return _size;
        }
        public double get (int n) {
            Util.enforce (n < _size);
            return _data [n];
        }
        public void push (double num) {
            if (_size == _data.length) {
                reserve ((int) (size () * 1.5));
            }
            _data [_size] = num;
            ++_size;
        }

        // Copy out data to a new array
        public double[] toArray () {
            double[] res = new double [size()];
            System.arraycopy (_data, 0, res, 0, size());
            return res;
        }


        // Expand capacity to len
        public void reserve (int len) {
            Util.enforce (len >= size ());
            double[] newData = new double [len];
            System.arraycopy (_data, 0, newData, 0, _size);
            _data = newData;
        }
    }

    public final static class Float {
        private final int START_CAPACITY = 8;
        private float[] _data = new float [START_CAPACITY];
        private int      _size = 0;  // num of currently stored elems

        public int size () {
            return _size;
        }
        public float get (int n) {
            Util.enforce (n < _size);
            return _data [n];
        }
        public void push (float num) {
            if (_size == _data.length) {
                reserve ((int) (size () * 1.5));
            }
            _data [_size] = num;
            ++_size;
        }

        // Copy out data to a new array
        public float[] toArray () {
            float[] res = new float [size()];
            System.arraycopy (_data, 0, res, 0, size());
            return res;
        }


        // Expand capacity to len
        public void reserve (int len) {
            Util.enforce (len >= size ());
            float[] newData = new float [len];
            System.arraycopy (_data, 0, newData, 0, _size);
            _data = newData;
        }
    }

}
            
        
