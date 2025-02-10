#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>
using namespace std;

// This is a quick-and-dirty implementation of arbitrary-size integers
// solely to demonstrate a divide-and-conquer multiplication method
// in CPSC 320.  The code is written for simplicity and not efficiency,
// so we don't recommend that you use this in real life!
class BigNum320 {
public: // Super horrible kludge!  Make everything public for easy programming.
        // Do NOT do this in real life!!!

    static const int radix = 10;        // Base for the numbers.
                                        // Make sure radix*radix < max integer

    vector<int> digits;                 // Array of the digits
                                        //   Stored low-order first
                                        //   i.e., digits[0] is the one's digit

    // ----- Constructors ------------------------------------
    // Creates a BigNum320 with n digits, all initialized to 0
    BigNum320(int n) {
        digits.assign(n,0);
    }

    // ----- Arithmetic Functions ----------------------------

    // add(x,y):  Addition, returns x+y
    // Output size is 1 digit larger than max of input sizes
    // to make room for carry.
    static BigNum320 *add(BigNum320 *x, BigNum320 *y) {
        int smallern = min(x->digits.size(),y->digits.size());
        int biggern = max(x->digits.size(),y->digits.size());
        BigNum320 *result = new BigNum320(biggern+1);

        int carry=0;  // This will store any carry from previous digit
        int i;
        for (i=0; i < smallern; i++) {
            // Up to the smaller size, we have digits from both x and y to add.
            int digit_sum = (x->digits[i] + y->digits[i]) + carry;
            result->digits[i] = digit_sum % radix;
            carry = digit_sum / radix;
        }
        // After this point, we just need to propagate any carry.
        if (x->digits.size() > smallern) {
            // x is the number with more digits
            for (i=smallern; i < biggern; i++) {
                int digit_sum = x->digits[i] + carry;
                result->digits[i] = digit_sum % radix;
                carry = digit_sum / radix;
            }
        } else {
            // y is the number with more digits
            for (i=smallern; i < biggern; i++) {
                int digit_sum = y->digits[i] + carry;
                result->digits[i] = digit_sum % radix;
                carry = digit_sum / radix;
            }
        }
        result->digits[biggern] = carry;
        return result;
    }

    // sub(x,y):  Subtraction, returns x-y
    // Output size is the size of the number being subtracted from (x),
    // which is assumed to be big enough to handle any borrows.
    // Also assumes that x>=y, so the result x-y is nonnegative.
    static BigNum320 *sub(BigNum320 *x, BigNum320 *y) {
        int xsize = x->digits.size();
        int ysize = y->digits.size();
        BigNum320 *result = new BigNum320(xsize);

        int borrow=0;  // This will store any borrow from previous digit
        int i;
        for (i=0; i < ysize; i++) {
            // Up to the size of y, we have digits from y to subtract.
            int digit_sum = (x->digits[i] - y->digits[i]) - borrow;
            if (digit_sum < 0) {
                digit_sum += radix;
                borrow=1;
            } else {
                borrow=0;
            }
            result->digits[i] = digit_sum;
        }
        // After this point, we just need to propagate any borrow.
        for (i=ysize; i < xsize; i++) {
            int digit_sum = x->digits[i] - borrow;
            if (digit_sum < 0) {
                digit_sum += radix;
                borrow=1;
            } else {
                borrow=0;
            }
            result->digits[i] = digit_sum;
        }
        // This should never happen, but worth checking
        if (borrow!=0) {
            cout << "Error: Subtraction underflow.\n"; cout.flush();
            // I should throw an exception, but too lazy.
        }
        return result;
    }


    // ----- Specialized Helper Functions --------------------

    // bottomDigits(x,k):
    // Returns the bottom k digits of x.
    // Fills in high-order zeroes if x->digits.size() < k
    static BigNum320 *bottomDigits(BigNum320 *x, int k) {
        BigNum320 *result = new BigNum320(k);
        for (int i=0; i < k; i++) {
            if (i < x->digits.size()) {
                // Expected Case.  Copy the digit over.
                result->digits[i] = x->digits[i];
            } else {
                // We're past the end of x.
                result->digits[i] = 0;
            }
        }
        return result;
    }
    
    // topDigits(x,k):
    // Returns the top digits of x after the BOTTOM k are removed..
    // If x->digits.size() <= k, returns a 1-digit 0.
    static BigNum320 *topDigits(BigNum320 *x, int k) {
        // Vacuous case when k is too big.
        if (x->digits.size() <= k) return new BigNum320(1);

        // OK, normal case, determine the size and copy over the digits.
        int size = x->digits.size() - k;
        BigNum320 *result = new BigNum320(size);
        for (int i=0; i < size; i++) {
            result->digits[i] = x->digits[k+i];
        }
        return result;
    }
    
    // shiftAccumulate(x,i,y):
    //                         returns void, modifies y
    // Adds the value of x, shifted left by i digits, to y.
    // The effect is y := y + x*radix^i
    static void shiftAccumulate(BigNum320 *x, int i, BigNum320 *y) {
        int carry=0;  // This will store any carry from previous digit
        int j;
        for (j=0; j < x->digits.size(); j++) {
            int digit_sum = (y->digits[i+j] + x->digits[j]) + carry;
            y->digits[i+j] = digit_sum % radix;
            carry = digit_sum / radix;
        }
        // In general, may need to propagate carry further.
        while (carry!=0) {
            // This should never happen, but good to check
            if (i+j >= y->digits.size()) {
                cout << "Error:  Overflow in shiftAccumulate.\n";
                break;  // I should properly throw an exception, but too lazy.
            }

            int digit_sum = y->digits[i+j] + carry;
            y->digits[i+j] = digit_sum % radix;
            carry = digit_sum / radix;
            j++;
        }
    }


    // ----- Multiplication Functions ----------------------
    // Output size will be double the max of the input sizes.

    // mult1
    //          Implements the paper-and-pencil algorithm.
    //          This is also the base case for small sizes of mult2 and mult3.
    static BigNum320 *mult1(BigNum320 *x, BigNum320 *y) {
        BigNum320 *result = new BigNum320(2*max(x->digits.size(),y->digits.size()));

        BigNum320 *pp = new BigNum320(x->digits.size()+1); // partial product
        for (int i=0; i < y->digits.size(); i++) {
            // Working from the low-order digits of y,
            // we multiply x by the ith digit of y...
            int carry=0;  // This will store any carry from previous digit
            for (int j=0; j < x->digits.size(); j++) {
                int digit_product = (x->digits[j] * y->digits[i]) + carry;
                pp->digits[j] = digit_product % radix;
                carry = digit_product / radix;
            }
            pp->digits[x->digits.size()] = carry;

            // Next, add the partial product to the result,
            // but shifted over i digits.
            shiftAccumulate(pp,i,result);
        }

        delete pp;
        return result;
    }

    // mult2
    //          Implements the obvious divide-and-conquer solution.
    static BigNum320 *mult2(BigNum320 *x, BigNum320 *y) {
        // Base Cases:  If x or y are small enough, don't bother recursing.
        //              Just use the n^2 algorithm in mult1.
        //              Note that I haven't tuned these cut-offs at all.
        if (x->digits.size() < 10) return mult1(x,y);
        if (y->digits.size() < 10) return mult1(x,y);

        // General Case
        int n = max(x->digits.size(), y->digits.size());
        int halfn = n/2;
        // Break up x and y into upper and lower halves:
        BigNum320 *xu = topDigits(x,halfn);
        BigNum320 *xl = bottomDigits(x,halfn);
        BigNum320 *yu = topDigits(y,halfn);
        BigNum320 *yl = bottomDigits(y,halfn);
        // Recursive calls to get the 4 products of the halves:
        BigNum320 *xuyu = mult2(xu,yu);
        BigNum320 *xuyl = mult2(xu,yl);
        BigNum320 *xlyu = mult2(xl,yu);
        BigNum320 *xlyl = mult2(xl,yl);
        // Prepare to add up the various partial products
        BigNum320 *result = new BigNum320(2*n);
        BigNum320 *xuylxlyu = add(xuyl, xlyu);
        shiftAccumulate(xlyl,0,result);
        shiftAccumulate(xuylxlyu,halfn,result);
        shiftAccumulate(xuyu,2*halfn,result);
        // Clean up memory.
        delete xu;
        delete xl;
        delete yu;
        delete yl;
        delete xuyu;
        delete xuyl;
        delete xlyu;
        delete xlyl;
        delete xuylxlyu;

        return result;
    }

    // mult3
    static BigNum320 *mult3(BigNum320 *x, BigNum320 *y) {
        // Base Cases:  If x or y are small enough, don't bother recursing.
        //              Just use the n^2 algorithm in mult1.
        //              Note that I haven't tuned these cut-offs at all.
        if (x->digits.size() < 10) return mult1(x,y);
        if (y->digits.size() < 10) return mult1(x,y);

        // General Case
        int n = max(x->digits.size(), y->digits.size());
        int halfn = n/2;
        // Break up x and y into upper and lower halves:
        BigNum320 *xu = topDigits(x,halfn);
        BigNum320 *xl = bottomDigits(x,halfn);
        BigNum320 *yu = topDigits(y,halfn);
        BigNum320 *yl = bottomDigits(y,halfn);
        // ----------------------------------------------------
        // YOUR CODE GOES HERE.
        // Do NOT make any changes outside of this one spot.
        // (You are also allowed to make changes in main() for testing and
        // timing.)
        
        // The next line is just a stub so the program will compile.
        // Feel free to use it or discard it.
        BigNum320 *result = new BigNum320(2*n);

        // END OF YOUR CODE GOES HERE REGION
        // Do not make changes after this point.
        // ----------------------------------------------------
        // Clean up memory allocted before the YOUR CODE HERE region.
        delete xu;
        delete xl;
        delete yu;
        delete yl;

        return result;
    }



    // ----- Helper Functions for Debugging -------------------------

    // Prints the number.  Printing is so low-order bit is rightmost
    void print() {
        for (int i = digits.size()-1; i>=0; i--)
            cout << digits[i];
    }
};

// This main routine is a simple testbench to try out the multiplication
// routines and time them.
int main() {
  int n;

  cout << "Enter number of digits: ";
  cin >> n;

  cout << "Generating two numbers to multiply...\n";
  BigNum320 *x = new BigNum320(n);
  BigNum320 *y = new BigNum320(n);
  for (int i=0; i< x->digits.size(); i++) x->digits[i] = i % x->radix;
  for (int i=0; i< y->digits.size(); i++) y->digits[i] = i % y->radix;

  // Testing code
  /*
  x->print(); cout << '\n';
  y->print(); cout << '\n';
  BigNum320 *z = BigNum320::mult2(x,y);
  z->print(); cout << '\n';
  delete z;
  */

  /*
  cout << "Testing multiplication functions...\n";
  BigNum320 *result1 = BigNum320::mult1(x,y);
  BigNum320 *result2 = BigNum320::mult2(x,y);
  if (result1->digits == result2->digits) cout << "mult2 PASSES.\n\n";
  else cout << "mult2 FAILS.\n\n";
  BigNum320 *result3 = BigNum320::mult3(x,y);
  if (result1->digits == result3->digits) cout << "mult3 PASSES.\n\n";
  else cout << "mult3 FAILS.\n\n";
  delete result1;
  delete result2;
  delete result3;
  */

  cout << "Timing mult1...\n";
  auto t0 = std::clock();
  delete BigNum320::mult1(x,y);
  auto t1 = std::clock();
  double duration = (t1-t0)/ (double) CLOCKS_PER_SEC;
  cout << "Done.\n";
  cout << "mult1 time = " << duration << " seconds.\n\n";

  /*
  cout << "Timing mult2...\n";
  t0 = std::clock();
  delete BigNum320::mult2(x,y);
  t1 = std::clock();
  duration = (t1-t0)/ (double) CLOCKS_PER_SEC;
  cout << "Done.\n";
  cout << "mult2 time = " << duration << " seconds.\n\n";
  */

  /*
  cout << "Timing mult3...\n";
  t0 = std::clock();
  delete BigNum320::mult3(x,y);
  t1 = std::clock();
  duration = (t1-t0)/ (double) CLOCKS_PER_SEC;
  cout << "Done.\n";
  cout << "mult3 time = " << duration << " seconds.\n\n";
  */

  delete x;
  delete y;
}
