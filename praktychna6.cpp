#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace std;

struct Stats {
    long long comparisons;
    long long swaps;
    long long heapifyCalls;
};

struct RunResult {
    Stats build;
    Stats extract;
    Stats total;
    bool ok;
};

void resetStats(Stats& s) {
    s.comparisons = 0;
    s.swaps = 0;
    s.heapifyCalls = 0;
}

int* copyArray(const int* src, int n) {
    if (n <= 0) {
        return nullptr;
    }
    int* dst = new int[n];
    for (int i = 0; i < n; ++i) {
        dst[i] = src[i];
    }
    return dst;
}

void printArrayInline(const int* arr, int n) {
    cout << "[";
    for (int i = 0; i < n; ++i) {
        cout << arr[i];
        if (i + 1 < n) {
            cout << ", ";
        }
    }
    cout << "]";
}

void printArrayWithSplit(const int* arr, int n, int heapSize) {
    cout << "купа=[";
    for (int i = 0; i < heapSize; ++i) {
        cout << arr[i];
        if (i + 1 < heapSize) {
            cout << ", ";
        }
    }
    cout << "] | відсортовано=[";
    for (int i = heapSize; i < n; ++i) {
        cout << arr[i];
        if (i + 1 < n) {
            cout << ", ";
        }
    }
    cout << "]";
}

int checkMaxHeap(const int* arr, int heapSize) {
    if (arr == nullptr || heapSize <= 1) {
        return -1;
    }
    int lastParent = heapSize / 2 - 1;
    for (int i = 0; i <= lastParent; ++i) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < heapSize && arr[i] < arr[left]) {
            return i;
        }
        if (right < heapSize && arr[i] < arr[right]) {
            return i;
        }
    }
    return -1;
}

bool checkExtractionStepInvariant(const int* arr, int n, int heapSize) {
    if (checkMaxHeap(arr, heapSize) != -1) {
        return false;
    }
    for (int i = heapSize; i + 1 < n; ++i) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    if (heapSize > 0 && heapSize < n) {
        for (int i = 0; i < heapSize; ++i) {
            if (arr[i] > arr[heapSize]) {
                return false;
            }
        }
    }
    return true;
}

void heapifyIterative(int* arr, int heapSize, int root, Stats& st) {
    st.heapifyCalls++;
    int current = root;
    while (true) {
        int largest = current;
        int left = 2 * current + 1;
        int right = 2 * current + 2;

        if (left < heapSize) {
            st.comparisons++;
            if (arr[left] > arr[largest]) {
                largest = left;
            }
        }
        if (right < heapSize) {
            st.comparisons++;
            if (arr[right] > arr[largest]) {
                largest = right;
            }
        }
        if (largest != current) {
            int tmp = arr[current];
            arr[current] = arr[largest];
            arr[largest] = tmp;
            st.swaps++;
            current = largest;
        }
        else {
            break;
        }
    }
}

void heapifyRecursiveStep(int* arr, int heapSize, int current, Stats& st) {
    int largest = current;
    int left = 2 * current + 1;
    int right = 2 * current + 2;

    if (left < heapSize) {
        st.comparisons++;
        if (arr[left] > arr[largest]) {
            largest = left;
        }
    }
    if (right < heapSize) {
        st.comparisons++;
        if (arr[right] > arr[largest]) {
            largest = right;
        }
    }
    if (largest != current) {
        int tmp = arr[current];
        arr[current] = arr[largest];
        arr[largest] = tmp;
        st.swaps++;
        heapifyRecursiveStep(arr, heapSize, largest, st);
    }
}

void heapifyRecursive(int* arr, int heapSize, int root, Stats& st) {
    st.heapifyCalls++;
    heapifyRecursiveStep(arr, heapSize, root, st);
}

void buildMaxHeap(int* arr, int n, Stats& buildSt, bool useRecursive, bool trace) {
    resetStats(buildSt);
    if (arr == nullptr || n <= 1) {
        return;
    }
    int startIdx = n / 2 - 1;
    if (trace) {
        cout << "Етап побудови максимальної купи (останній внутрішній вузол i = " << startIdx << "):\n";
    }
    for (int i = startIdx; i >= 0; --i) {
        if (useRecursive) {
            heapifyRecursive(arr, n, i, buildSt);
        }
        else {
            heapifyIterative(arr, n, i, buildSt);
        }
        if (trace) {
            cout << "  Вузол i=" << i << ", активний розмір=" << n << ", масив=";
            printArrayInline(arr, n);
            cout << "\n";
        }
    }
}

bool verifySortedAndComposition(const int* orig, int origN, const int* sorted, int sortedN) {
    if (origN != sortedN) {
        return false;
    }
    if (sortedN <= 0) {
        return true;
    }
    for (int i = 0; i + 1 < sortedN; ++i) {
        if (sorted[i] > sorted[i + 1]) {
            return false;
        }
    }
    long long sumOrig = 0;
    long long sumSorted = 0;
    long long sqOrig = 0;
    long long sqSorted = 0;
    for (int i = 0; i < sortedN; ++i) {
        long long a = orig[i];
        long long b = sorted[i];
        sumOrig += a;
        sumSorted += b;
        sqOrig += a * a;
        sqSorted += b * b;
    }
    return (sumOrig == sumSorted) && (sqOrig == sqSorted);
}

RunResult heapSort(int* arr, int n, bool useRecursive, bool trace) {
    RunResult res;
    resetStats(res.build);
    resetStats(res.extract);
    resetStats(res.total);
    res.ok = true;

    int* origCopy = copyArray(arr, n);
    bool doTrace = trace && (n > 0 && n <= 20);

    buildMaxHeap(arr, n, res.build, useRecursive, doTrace);

    if (checkMaxHeap(arr, n) != -1) {
        res.ok = false;
    }

    if (doTrace) {
        cout << "Стан після побудови купи: ";
        printArrayInline(arr, n);
        cout << " (перевірка купи: " << (checkMaxHeap(arr, n) == -1 ? "успішно" : "помилка") << ")\n";
        cout << "Етап вилучення найбільших елементів:\n";
    }

    int stepNum = 0;
    for (int end = n - 1; end > 0; --end) {
        int tmp = arr[0];
        arr[0] = arr[end];
        arr[end] = tmp;
        res.extract.swaps++;

        if (useRecursive) {
            heapifyRecursive(arr, end, 0, res.extract);
        }
        else {
            heapifyIterative(arr, end, 0, res.extract);
        }

        stepNum++;
        bool stepOk = checkExtractionStepInvariant(arr, n, end);
        if (!stepOk) {
            res.ok = false;
        }
        if (doTrace) {
            cout << "  Крок вилучення #" << stepNum << " (вилучено " << arr[end] << ", розмір купи=" << end << "): ";
            printArrayWithSplit(arr, n, end);
            cout << " -> перевірка: " << (stepOk ? "успішно" : "помилка") << "\n";
        }
    }

    res.total.comparisons = res.build.comparisons + res.extract.comparisons;
    res.total.swaps = res.build.swaps + res.extract.swaps;
    res.total.heapifyCalls = res.build.heapifyCalls + res.extract.heapifyCalls;

    if (!verifySortedAndComposition(origCopy, n, arr, n)) {
        res.ok = false;
    }

    delete[] origCopy;
    return res;
}

void runAutomatedTests() {
    cout << "=== Автоматизовані тести HeapSort ===\n";
    const int testCount = 14;
    const char* names[testCount] = {
        "тест 1  порожній масив",
        "тест 2  один елемент",
        "тест 3  два впорядковані",
        "тест 4  два переставлені",
        "тест 5  зростання",
        "тест 6  спадання",
        "тест 7  змішаний порядок",
        "тест 8  повторення",
        "тест 9  від'ємні числа",
        "тест 10 усі рівні",
        "тест 11 комбінований випадок",
        "тест 12 неповний останній рівень",
        "тест 13 власний: непарний зі змішаними",
        "тест 14 власний: масив варіанта 22"
    };

    int t0[] = { 0 };
    int t1[] = { 7 };
    int t2[] = { 1, 2 };
    int t3[] = { 2, 1 };
    int t4[] = { 1, 2, 3, 4, 5 };
    int t5[] = { 5, 4, 3, 2, 1 };
    int t6[] = { 4, 1, 5, 2, 3 };
    int t7[] = { 3, 1, 3, 2, 1 };
    int t8[] = { 0, -5, 8, -2, 4 };
    int t9[] = { 6, 6, 6, 6 };
    int t10[] = { 9, -1, 9, 0, -1, 5 };
    int t11[] = { 10, 7, 8, 4, 2, 1, 3 };
    int t12[] = { 14, -3, 27, 0, 14, -8, 5 };
    int t13[] = { 44, 22, 33, 17, 40, 25, 19, 42, 15, 31, 23, 37, 20, 29, 18 };

    const int* arrays[testCount] = { t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13 };
    int sizes[testCount] = { 0, 1, 2, 2, 5, 5, 5, 5, 5, 4, 6, 7, 7, 15 };

    int passedSort = 0;
    int passedBuild = 0;
    int totalBuildChecks = 0;

    for (int i = 0; i < testCount; ++i) {
        int n = sizes[i];
        int* aIter = copyArray(arrays[i], n);
        int* aRec = copyArray(arrays[i], n);

        bool buildOk = true;
        if (n >= 5) {
            totalBuildChecks++;
            Stats bs;
            int* bCopy = copyArray(arrays[i], n);
            buildMaxHeap(bCopy, n, bs, false, false);
            int violation = checkMaxHeap(bCopy, n);
            buildOk = (violation == -1);
            if (buildOk) {
                passedBuild++;
            }
            delete[] bCopy;
        }

        RunResult rIter = heapSort(aIter, n, false, false);
        RunResult rRec = heapSort(aRec, n, true, false);
        bool sameStats = (rIter.total.comparisons == rRec.total.comparisons) &&
            (rIter.total.swaps == rRec.total.swaps);
        bool allOk = rIter.ok && rRec.ok && buildOk && sameStats;
        if (allOk) {
            passedSort++;
        }

        cout << left << setw(38) << names[i]
            << " побудова: " << setw(11) << (n >= 5 ? (buildOk ? "успішно" : "помилка") : "н/д (<5)")
            << " сортування: " << (allOk ? "успішно" : "помилка") << "\n";

        delete[] aIter;
        delete[] aRec;
    }

    int extraBuildArr[] = { 6, 6, 6, 6, 6 };
    Stats extraStats;
    buildMaxHeap(extraBuildArr, 5, extraStats, false, false);
    totalBuildChecks++;
    if (checkMaxHeap(extraBuildArr, 5) == -1) {
        passedBuild++;
    }

    cout << "\n=== Перевірка функції властивості максимальної купи ===\n";
    int hValid1[] = { 30, 25, 20, 10, 15, 8, 12 };
    int hValid2[] = { 50, 40, 45, 40, 35, 20 };
    int hValid3[] = { 9, 9, 9, 9, 9 };
    int hInvalid1[] = { 20, 25, 18, 10, 12 };
    int hInvalid2[] = { 30, 18, 25, 19, 10, 12 };
    int hInvalid3[] = { 40, 35, 30, 20, 15, 32 };

    const int* heaps[6] = { hValid1, hValid2, hValid3, hInvalid1, hInvalid2, hInvalid3 };
    int hSizes[6] = { 7, 6, 5, 5, 6, 6 };
    bool expectedValid[6] = { true, true, true, false, false, false };
    const char* hLabels[6] = {
        "правильна піраміда 1",
        "правильна піраміда 2",
        "правильна піраміда 3 (рівні)",
        "неправильна піраміда 1",
        "неправильна піраміда 2",
        "неправильна піраміда 3"
    };

    int passedHeapChecks = 0;
    for (int i = 0; i < 6; ++i) {
        int idx = checkMaxHeap(heaps[i], hSizes[i]);
        bool isValid = (idx == -1);
        bool testOk = (isValid == expectedValid[i]);
        if (testOk) {
            passedHeapChecks++;
        }
        cout << left << setw(31) << hLabels[i] << " масив=";
        printArrayInline(heaps[i], hSizes[i]);
        if (idx == -1) {
            cout << " -> коректна max-heap  [" << (testOk ? "успішно" : "помилка") << "]\n";
        }
        else {
            cout << " -> порушення у вузлі i=" << idx << " [" << (testOk ? "успішно" : "помилка") << "]\n";
        }
    }

    cout << "\nУспішних перевірок сортування: " << passedSort << " з " << testCount << "\n";
    cout << "Успішних перевірок побудови купи: " << passedBuild << " з " << totalBuildChecks << "\n";
    cout << "Успішних перевірок властивості купи: " << passedHeapChecks << " з 6\n";
}

void runVariant22() {
    cout << "=== Індивідуальний варіант 22 ===\n";
    const int n = 15;
    int initArr[n] = { 44, 22, 33, 17, 40, 25, 19, 42, 15, 31, 23, 37, 20, 29, 18 };
    cout << "Початковий масив: ";
    printArrayInline(initArr, n);
    cout << "\n\n";

    int* arrIter = copyArray(initArr, n);
    RunResult rIter = heapSort(arrIter, n, false, true);

    cout << "\nОстаточний відсортований масив: ";
    printArrayInline(arrIter, n);
    cout << "\n\nСтатистика за етапами (ітеративне відновлення купи):\n";
    cout << left << setw(16) << "Етап"
        << right << setw(14) << "Порівняння"
        << setw(12) << "Обміни"
        << setw(22) << "Відновлення купи" << "\n";
    cout << left << setw(16) << "Побудова"
        << right << setw(14) << rIter.build.comparisons
        << setw(12) << rIter.build.swaps
        << setw(22) << rIter.build.heapifyCalls << "\n";
    cout << left << setw(16) << "Вилучення"
        << right << setw(14) << rIter.extract.comparisons
        << setw(12) << rIter.extract.swaps
        << setw(22) << rIter.extract.heapifyCalls << "\n";
    cout << left << setw(16) << "Усього"
        << right << setw(14) << rIter.total.comparisons
        << setw(12) << rIter.total.swaps
        << setw(22) << rIter.total.heapifyCalls << "\n";
    cout << "Автоматична перевірка результату: " << (rIter.ok ? "успішно" : "помилка") << "\n";

    int* arrRec = copyArray(initArr, n);
    RunResult rRec = heapSort(arrRec, n, true, false);
    cout << "Контроль рекурсивного варіанту: порівняння=" << rRec.total.comparisons
        << ", обміни=" << rRec.total.swaps
        << ", виклики=" << rRec.total.heapifyCalls
        << ", перевірка=" << (rRec.ok ? "успішно" : "помилка") << "\n";

    delete[] arrIter;
    delete[] arrRec;
}

void fillDataset(int* arr, int n, int typeIdx) {
    if (typeIdx == 0) {
        for (int i = 0; i < n; ++i) {
            arr[i] = i + 1;
        }
    }
    else if (typeIdx == 1) {
        for (int i = 0; i < n; ++i) {
            arr[i] = n - i;
        }
    }
    else if (typeIdx == 2) {
        srand(22);
        for (int i = 0; i < n; ++i) {
            arr[i] = 1 + rand() % (10 * n);
        }
    }
    else {
        srand(22);
        for (int i = 0; i < n; ++i) {
            arr[i] = 1 + rand() % 5;
        }
    }
}

void runExperiment() {
    cout << "=== Порівняльний експеримент HeapSort (12 запусків) ===\n";
    int sizes[3] = { 100, 1000, 10000 };
    const char* typeNames[4] = {
        "Зростання",
        "Спадання",
        "Змішаний",
        "Багато однакових"
    };

    cout << left << setw(8) << "Розмір"
        << setw(19) << "Тип даних"
        << right << setw(15) << "Поб. порівн."
        << setw(14) << "Поб. обміни"
        << setw(16) << "Усього порівн."
        << setw(15) << "Усього обміни"
        << setw(12) << "Перевірка" << "\n";

    int okRuns = 0;
    for (int s = 0; s < 3; ++s) {
        int n = sizes[s];
        int* base = new int[n];
        for (int t = 0; t < 4; ++t) {
            fillDataset(base, n, t);
            int* work = copyArray(base, n);
            RunResult res = heapSort(work, n, false, false);
            if (res.ok) {
                okRuns++;
            }
            cout << left << setw(8) << n
                << setw(19) << typeNames[t]
                << right << setw(15) << res.build.comparisons
                << setw(14) << res.build.swaps
                << setw(16) << res.total.comparisons
                << setw(15) << res.total.swaps
                << setw(12) << (res.ok ? "успішно" : "помилка") << "\n";
            delete[] work;
        }
        delete[] base;
    }
    cout << "\nУспішних запусків експерименту: " << okRuns << " з 12\n\n";

    cout << "=== Дослідження: ітеративне та рекурсивне відновлення купи (n = 10000) ===\n";
    cout << left << setw(19) << "Тип даних"
        << right << setw(14) << "Ітер. порівн."
        << setw(13) << "Ітер. обміни"
        << setw(14) << "Рек. порівн."
        << setw(13) << "Рек. обміни"
        << setw(15) << "Різниця" << "\n";

    int nComp = 10000;
    int* baseComp = new int[nComp];
    for (int t = 0; t < 4; ++t) {
        fillDataset(baseComp, nComp, t);
        int* wIter = copyArray(baseComp, nComp);
        int* wRec = copyArray(baseComp, nComp);
        RunResult rI = heapSort(wIter, nComp, false, false);
        RunResult rR = heapSort(wRec, nComp, true, false);
        bool identical = (rI.total.comparisons == rR.total.comparisons) &&
            (rI.total.swaps == rR.total.swaps) &&
            (rI.total.heapifyCalls == rR.total.heapifyCalls);
        cout << left << setw(19) << typeNames[t]
            << right << setw(14) << rI.total.comparisons
            << setw(13) << rI.total.swaps
            << setw(14) << rR.total.comparisons
            << setw(13) << rR.total.swaps
            << setw(15) << (identical ? "0 (ідентично)" : "є різниця") << "\n";
        delete[] wIter;
        delete[] wRec;
    }
    delete[] baseComp;
}

void runCustomInput() {
    cout << "Введіть кількість елементів масиву: ";
    int n;
    if (!(cin >> n) || n < 0) {
        cout << "Некоректний розмір масиву.\n";
        return;
    }
    int* arr = copyArray(nullptr, 0);
    if (n > 0) {
        arr = new int[n];
        cout << "Введіть " << n << " цілих чисел через пробіл: ";
        for (int i = 0; i < n; ++i) {
            cin >> arr[i];
        }
    }
    RunResult res = heapSort(arr, n, false, n <= 20);
    cout << "Відсортований масив: ";
    printArrayInline(arr, n);
    cout << "\nПобудова: порівняння=" << res.build.comparisons << ", обміни=" << res.build.swaps
        << ", виклики=" << res.build.heapifyCalls << "\n";
    cout << "Вилучення: порівняння=" << res.extract.comparisons << ", обміни=" << res.extract.swaps
        << ", виклики=" << res.extract.heapifyCalls << "\n";
    cout << "Усього: порівняння=" << res.total.comparisons << ", обміни=" << res.total.swaps
        << ", виклики=" << res.total.heapifyCalls << "\n";
    cout << "Перевірка: " << (res.ok ? "успішно" : "помилка") << "\n";
    delete[] arr;
}

void printMenu() {
    cout << "=== Сортування HeapSort ===\n";
    cout << "1. Введення масиву і сортування\n";
    cout << "2. Автоматизовані тести\n";
    cout << "3. Індивідуальний варіант 22\n";
    cout << "4. Порівняльний експеримент\n";
    cout << "0. Вихід\n";
}

int main() {
    int mode = 0;
    if (cin >> mode) {
        if (mode == -1) {
            printMenu();
            return 0;
        }
        if (mode == 1) {
            runCustomInput();
            return 0;
        }
        if (mode == 2) {
            runAutomatedTests();
            return 0;
        }
        if (mode == 3) {
            runVariant22();
            return 0;
        }
        if (mode == 4) {
            runExperiment();
            return 0;
        }
    }

    int choice = -1;
    do {
        printMenu();
        cout << "Оберіть пункт меню: ";
        if (!(cin >> choice)) {
            break;
        }
        if (choice == 1) {
            runCustomInput();
        }
        else if (choice == 2) {
            runAutomatedTests();
        }
        else if (choice == 3) {
            runVariant22();
        }
        else if (choice == 4) {
            runExperiment();
        }
    } while (choice != 0);

    return 0;
}