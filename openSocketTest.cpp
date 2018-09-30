#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    bool arr[] = {true, false, true, true, true, false, true};
    bool one = false;
    bool two = false;
    bool three = false;
    int counter = 0;

    int oneint = 0;
    int twoint = 0;
    int threeint = 0;

    cout << "?" << endl;
    for(int i = 0; i < 7; i++) {

      counter ++;
      cout << "counter: " << counter << endl;
      cout << "arr: " << arr[i] << endl;
      cout << "i: " << i << endl;
        if(arr[i]) {
           if(counter%3 == 0) {
             cout << "setting ports" << endl;
             three = arr[i];
             //cout << "i: " << i <<" arri: " << arr[i];
             two = arr[i-1];
             one = arr[i-2];
             cout <<  one << two << three << endl;

             threeint = i;
             twoint = i-1;
             oneint = i-2;

             cout << oneint << " " << twoint << " " << threeint << endl;
             break;
           }
        }
        else {
          counter = 0;
        }
    }

    if(one && two && three) {
        cout << "worked" << endl;
        cout << oneint << " " << twoint << " " << threeint << endl;
    }
    else {
        cout << "did not work" << endl;
    }

    return 0;
}
