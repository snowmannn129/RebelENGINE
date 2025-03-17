export class Vector2 {
    constructor(
        public x: number = 0,
        public y: number = 0
    ) {}

    public static distance(a: Vector2, b: Vector2): number {
        const dx = a.x - b.x;
        const dy = a.y - b.y;
        return Math.sqrt(dx * dx + dy * dy);
    }

    public static lerp(a: Vector2, b: Vector2, t: number): Vector2 {
        t = Math.max(0, Math.min(1, t));
        return new Vector2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    public static angle(a: Vector2, b: Vector2): number {
        return Math.acos(
            Math.min(1, Math.max(-1, a.normalize().dot(b.normalize())))
        );
    }

    public clone(): Vector2 {
        return new Vector2(this.x, this.y);
    }

    public add(v: Vector2): Vector2 {
        return new Vector2(
            this.x + v.x,
            this.y + v.y
        );
    }

    public subtract(v: Vector2): Vector2 {
        return new Vector2(
            this.x - v.x,
            this.y - v.y
        );
    }

    public multiply(value: number | Vector2): Vector2 {
        if (value instanceof Vector2) {
            return new Vector2(
                this.x * value.x,
                this.y * value.y
            );
        }
        return new Vector2(
            this.x * value,
            this.y * value
        );
    }

    public divide(scalar: number): Vector2 {
        if (scalar === 0) {
            throw new Error('Division by zero');
        }
        return new Vector2(
            this.x / scalar,
            this.y / scalar
        );
    }

    public length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }

    public normalize(): Vector2 {
        const len = this.length();
        if (len === 0) {
            return new Vector2();
        }
        return this.divide(len);
    }

    public dot(v: Vector2): number {
        return this.x * v.x + this.y * v.y;
    }

    public reflect(normal: Vector2): Vector2 {
        const d = this.dot(normal);
        return new Vector2(
            this.x - 2 * d * normal.x,
            this.y - 2 * d * normal.y
        );
    }

    public project(onto: Vector2): Vector2 {
        const normalized = onto.normalize();
        const dot = this.dot(normalized);
        return normalized.multiply(dot);
    }

    public distanceTo(v: Vector2): number {
        return Vector2.distance(this, v);
    }

    public angleTo(v: Vector2): number {
        return Vector2.angle(this, v);
    }

    public lerp(target: Vector2, t: number): Vector2 {
        return Vector2.lerp(this, target, t);
    }
}

export class Vec3 {
    constructor(
        public x: number = 0,
        public y: number = 0,
        public z: number = 0
    ) {}

    static normalize(v: Vec3): Vec3 {
        return v.normalize();
    }

    static scale(v: Vec3, scalar: number): Vec3 {
        return v.multiply(scalar);
    }

    static distance(a: Vec3, b: Vec3): number {
        const dx = a.x - b.x;
        const dy = a.y - b.y;
        const dz = a.z - b.z;
        return Math.sqrt(dx * dx + dy * dy + dz * dz);
    }

    static lerp(a: Vec3, b: Vec3, t: number): Vec3 {
        t = Math.max(0, Math.min(1, t));
        return new Vec3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

    static angle(a: Vec3, b: Vec3): number {
        return Math.acos(
            Math.min(1, Math.max(-1, a.normalize().dot(b.normalize())))
        );
    }

    public clone(): Vec3 {
        return new Vec3(this.x, this.y, this.z);
    }

    public add(v: Vec3): Vec3 {
        return new Vec3(
            this.x + v.x,
            this.y + v.y,
            this.z + v.z
        );
    }

    public subtract(v: Vec3): Vec3 {
        return new Vec3(
            this.x - v.x,
            this.y - v.y,
            this.z - v.z
        );
    }

    public multiply(value: number | Vec3): Vec3 {
        if (value instanceof Vec3) {
            return new Vec3(
                this.x * value.x,
                this.y * value.y,
                this.z * value.z
            );
        }
        return new Vec3(
            this.x * value,
            this.y * value,
            this.z * value
        );
    }

    public divide(scalar: number): Vec3 {
        if (scalar === 0) {
            throw new Error('Division by zero');
        }
        return new Vec3(
            this.x / scalar,
            this.y / scalar,
            this.z / scalar
        );
    }

    public length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
    }

    public normalize(): Vec3 {
        const len = this.length();
        if (len === 0) {
            return new Vec3();
        }
        return this.divide(len);
    }

    public dot(v: Vec3): number {
        return this.x * v.x + this.y * v.y + this.z * v.z;
    }

    public cross(v: Vec3): Vec3 {
        return new Vec3(
            this.y * v.z - this.z * v.y,
            this.z * v.x - this.x * v.z,
            this.x * v.y - this.y * v.x
        );
    }

    public reflect(normal: Vec3): Vec3 {
        const d = this.dot(normal);
        return new Vec3(
            this.x - 2 * d * normal.x,
            this.y - 2 * d * normal.y,
            this.z - 2 * d * normal.z
        );
    }

    public project(onto: Vec3): Vec3 {
        const normalized = onto.normalize();
        const dot = this.dot(normalized);
        return normalized.multiply(dot);
    }

    public distanceTo(v: Vec3): number {
        return Vec3.distance(this, v);
    }

    public angleTo(v: Vec3): number {
        return Vec3.angle(this, v);
    }

    public lerp(target: Vec3, t: number): Vec3 {
        return Vec3.lerp(this, target, t);
    }
}

export class Mat4 {
    private elements: number[];

    constructor() {
        this.elements = [
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ];
    }

    static ortho(left: number, right: number, bottom: number, top: number, near: number, far: number): Mat4 {
        return Mat4.orthographic(left, right, bottom, top, near, far);
    }

    static multiply(a: Mat4, b: Mat4): Mat4 {
        return a.multiply(b);
    }

    public static lookAt(eye: Vec3, target: Vec3, up: Vec3): Mat4 {
        const mat = new Mat4();
        const zAxis = eye.subtract(target).normalize();
        const xAxis = up.cross(zAxis).normalize();
        const yAxis = zAxis.cross(xAxis);

        mat.elements[0] = xAxis.x;
        mat.elements[1] = xAxis.y;
        mat.elements[2] = xAxis.z;
        mat.elements[4] = yAxis.x;
        mat.elements[5] = yAxis.y;
        mat.elements[6] = yAxis.z;
        mat.elements[8] = zAxis.x;
        mat.elements[9] = zAxis.y;
        mat.elements[10] = zAxis.z;
        mat.elements[12] = -xAxis.dot(eye);
        mat.elements[13] = -yAxis.dot(eye);
        mat.elements[14] = -zAxis.dot(eye);

        return mat;
    }

    public static orthographic(
        left: number,
        right: number,
        bottom: number,
        top: number,
        near: number,
        far: number
    ): Mat4 {
        const mat = new Mat4();
        const w = right - left;
        const h = top - bottom;
        const d = far - near;

        mat.elements[0] = 2 / w;
        mat.elements[5] = 2 / h;
        mat.elements[10] = -2 / d;
        mat.elements[12] = -(right + left) / w;
        mat.elements[13] = -(top + bottom) / h;
        mat.elements[14] = -(far + near) / d;

        return mat;
    }

    public static identity(): Mat4 {
        return new Mat4();
    }

    public static translation(x: number, y: number, z: number): Mat4 {
        const mat = new Mat4();
        mat.elements[12] = x;
        mat.elements[13] = y;
        mat.elements[14] = z;
        return mat;
    }

    public static rotation(angleInRadians: number, axis: Vec3): Mat4 {
        const mat = new Mat4();
        const c = Math.cos(angleInRadians);
        const s = Math.sin(angleInRadians);
        const t = 1 - c;
        const x = axis.x, y = axis.y, z = axis.z;

        mat.elements[0] = t * x * x + c;
        mat.elements[1] = t * x * y + s * z;
        mat.elements[2] = t * x * z - s * y;

        mat.elements[4] = t * x * y - s * z;
        mat.elements[5] = t * y * y + c;
        mat.elements[6] = t * y * z + s * x;

        mat.elements[8] = t * x * z + s * y;
        mat.elements[9] = t * y * z - s * x;
        mat.elements[10] = t * z * z + c;

        return mat;
    }

    public static scaling(x: number, y: number, z: number): Mat4 {
        const mat = new Mat4();
        mat.elements[0] = x;
        mat.elements[5] = y;
        mat.elements[10] = z;
        return mat;
    }

    public static perspective(fov: number, aspect: number, near: number, far: number): Mat4 {
        const mat = new Mat4();
        const f = 1.0 / Math.tan(fov / 2);
        const nf = 1 / (near - far);

        mat.elements[0] = f / aspect;
        mat.elements[5] = f;
        mat.elements[10] = (far + near) * nf;
        mat.elements[11] = -1;
        mat.elements[14] = 2 * far * near * nf;
        mat.elements[15] = 0;

        return mat;
    }

    public multiply(other: Mat4): Mat4 {
        const result = new Mat4();
        const a = this.elements;
        const b = other.elements;
        const r = result.elements;

        for (let i = 0; i < 4; i++) {
            for (let j = 0; j < 4; j++) {
                r[i * 4 + j] = 
                    a[i * 4 + 0] * b[0 * 4 + j] +
                    a[i * 4 + 1] * b[1 * 4 + j] +
                    a[i * 4 + 2] * b[2 * 4 + j] +
                    a[i * 4 + 3] * b[3 * 4 + j];
            }
        }

        return result;
    }

    public getElement(index: number): number {
        return this.elements[index];
    }

    public getElements(): Float32Array {
        return new Float32Array(this.elements);
    }

    public transpose(): Mat4 {
        const result = new Mat4();
        for (let i = 0; i < 4; i++) {
            for (let j = 0; j < 4; j++) {
                result.elements[i * 4 + j] = this.elements[j * 4 + i];
            }
        }
        return result;
    }

    public determinant(): number {
        const e = this.elements;
        const n11 = e[0], n12 = e[4], n13 = e[8], n14 = e[12];
        const n21 = e[1], n22 = e[5], n23 = e[9], n24 = e[13];
        const n31 = e[2], n32 = e[6], n33 = e[10], n34 = e[14];
        const n41 = e[3], n42 = e[7], n43 = e[11], n44 = e[15];

        return (
            n41 * (
                n14 * n23 * n32 -
                n13 * n24 * n32 -
                n14 * n22 * n33 +
                n12 * n24 * n33 +
                n13 * n22 * n34 -
                n12 * n23 * n34
            ) +
            n42 * (
                n11 * n23 * n34 -
                n11 * n24 * n33 +
                n14 * n21 * n33 -
                n13 * n21 * n34 +
                n13 * n24 * n31 -
                n14 * n23 * n31
            ) +
            n43 * (
                n11 * n24 * n32 -
                n11 * n22 * n34 -
                n14 * n21 * n32 +
                n12 * n21 * n34 +
                n14 * n22 * n31 -
                n12 * n24 * n31
            ) +
            n44 * (
                n11 * n22 * n33 -
                n11 * n23 * n32 +
                n13 * n21 * n32 -
                n12 * n21 * n33 +
                n12 * n23 * n31 -
                n13 * n22 * n31
            )
        );
    }

    public inverse(): Mat4 | null {
        const det = this.determinant();
        if (det === 0) return null;

        const result = new Mat4();
        const e = this.elements;
        const r = result.elements;

        const n11 = e[0], n12 = e[4], n13 = e[8], n14 = e[12];
        const n21 = e[1], n22 = e[5], n23 = e[9], n24 = e[13];
        const n31 = e[2], n32 = e[6], n33 = e[10], n34 = e[14];
        const n41 = e[3], n42 = e[7], n43 = e[11], n44 = e[15];

        const invDet = 1 / det;

        r[0] = (n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44) * invDet;
        r[1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * invDet;
        r[2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * invDet;
        r[3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * invDet;
        r[4] = (n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44) * invDet;
        r[5] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * invDet;
        r[6] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * invDet;
        r[7] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * invDet;
        r[8] = (n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44) * invDet;
        r[9] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * invDet;
        r[10] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * invDet;
        r[11] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * invDet;
        r[12] = (n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34) * invDet;
        r[13] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * invDet;
        r[14] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * invDet;
        r[15] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * invDet;

        return result;
    }
}

export class Vector4 {
    constructor(
        public x: number = 0,
        public y: number = 0,
        public z: number = 0,
        public w: number = 0
    ) {}

    public static distance(a: Vector4, b: Vector4): number {
        const dx = a.x - b.x;
        const dy = a.y - b.y;
        const dz = a.z - b.z;
        const dw = a.w - b.w;
        return Math.sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
    }

    public static lerp(a: Vector4, b: Vector4, t: number): Vector4 {
        t = Math.max(0, Math.min(1, t));
        return new Vector4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

    public clone(): Vector4 {
        return new Vector4(this.x, this.y, this.z, this.w);
    }

    public add(v: Vector4): Vector4 {
        return new Vector4(
            this.x + v.x,
            this.y + v.y,
            this.z + v.z,
            this.w + v.w
        );
    }

    public subtract(v: Vector4): Vector4 {
        return new Vector4(
            this.x - v.x,
            this.y - v.y,
            this.z - v.z,
            this.w - v.w
        );
    }

    public multiply(scalar: number): Vector4 {
        return new Vector4(
            this.x * scalar,
            this.y * scalar,
            this.z * scalar,
            this.w * scalar
        );
    }

    public divide(scalar: number): Vector4 {
        if (scalar === 0) {
            throw new Error('Division by zero');
        }
        return new Vector4(
            this.x / scalar,
            this.y / scalar,
            this.z / scalar,
            this.w / scalar
        );
    }

    public length(): number {
        return Math.sqrt(
            this.x * this.x +
            this.y * this.y +
            this.z * this.z +
            this.w * this.w
        );
    }

    public normalize(): Vector4 {
        const len = this.length();
        if (len === 0) {
            return new Vector4();
        }
        return this.divide(len);
    }

    public dot(v: Vector4): number {
        return this.x * v.x + this.y * v.y + this.z * v.z + this.w * v.w;
    }

    public distanceTo(v: Vector4): number {
        return Vector4.distance(this, v);
    }

    public lerp(target: Vector4, t: number): Vector4 {
        return Vector4.lerp(this, target, t);
    }

    public equals(v: Vector4, tolerance: number = 0.0001): boolean {
        return (
            Math.abs(this.x - v.x) <= tolerance &&
            Math.abs(this.y - v.y) <= tolerance &&
            Math.abs(this.z - v.z) <= tolerance &&
            Math.abs(this.w - v.w) <= tolerance
        );
    }
}

export class Matrix4x4 {
    private static readonly IDENTITY_ELEMENTS = [
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    ];

    private elements: Float32Array;

    constructor() {
        this.elements = new Float32Array(Matrix4x4.IDENTITY_ELEMENTS);
    }

    public static identity(): Matrix4x4 {
        return new Matrix4x4();
    }

    public static translation(x: number, y: number, z: number): Matrix4x4 {
        const mat = new Matrix4x4();
        mat.elements[12] = x;
        mat.elements[13] = y;
        mat.elements[14] = z;
        return mat;
    }

    public static rotation(angleInRadians: number, axis: Vec3): Matrix4x4 {
        const mat = new Matrix4x4();
        const c = Math.cos(angleInRadians);
        const s = Math.sin(angleInRadians);
        const t = 1 - c;
        const x = axis.x, y = axis.y, z = axis.z;

        mat.elements[0] = t * x * x + c;
        mat.elements[1] = t * x * y + s * z;
        mat.elements[2] = t * x * z - s * y;

        mat.elements[4] = t * x * y - s * z;
        mat.elements[5] = t * y * y + c;
        mat.elements[6] = t * y * z + s * x;

        mat.elements[8] = t * x * z + s * y;
        mat.elements[9] = t * y * z - s * x;
        mat.elements[10] = t * z * z + c;

        return mat;
    }

    public static scaling(x: number, y: number, z: number): Matrix4x4 {
        const mat = new Matrix4x4();
        mat.elements[0] = x;
        mat.elements[5] = y;
        mat.elements[10] = z;
        return mat;
    }

    public multiply(other: Matrix4x4): Matrix4x4 {
        const result = new Matrix4x4();
        const a = this.elements;
        const b = other.elements;
        const r = result.elements;

        // Unrolled matrix multiplication for better performance
        r[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
        r[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
        r[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
        r[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

        r[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
        r[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
        r[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
        r[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

        r[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
        r[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
        r[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
        r[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

        r[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
        r[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
        r[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
        r[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

        return result;
    }

    public transpose(): Matrix4x4 {
        const result = new Matrix4x4();
        const r = result.elements;
        const e = this.elements;

        r[0] = e[0];  r[4] = e[1];  r[8] = e[2];   r[12] = e[3];
        r[1] = e[4];  r[5] = e[5];  r[9] = e[6];   r[13] = e[7];
        r[2] = e[8];  r[6] = e[9];  r[10] = e[10]; r[14] = e[11];
        r[3] = e[12]; r[7] = e[13]; r[11] = e[14]; r[15] = e[15];

        return result;
    }

    public determinant(): number {
        const e = this.elements;
        
        const n11 = e[0], n12 = e[4], n13 = e[8], n14 = e[12];
        const n21 = e[1], n22 = e[5], n23 = e[9], n24 = e[13];
        const n31 = e[2], n32 = e[6], n33 = e[10], n34 = e[14];
        const n41 = e[3], n42 = e[7], n43 = e[11], n44 = e[15];

        return (
            n41 * (
                n14 * n23 * n32 -
                n13 * n24 * n32 -
                n14 * n22 * n33 +
                n12 * n24 * n33 +
                n13 * n22 * n34 -
                n12 * n23 * n34
            ) +
            n42 * (
                n11 * n23 * n34 -
                n11 * n24 * n33 +
                n14 * n21 * n33 -
                n13 * n21 * n34 +
                n13 * n24 * n31 -
                n14 * n23 * n31
            ) +
            n43 * (
                n11 * n24 * n32 -
                n11 * n22 * n34 -
                n14 * n21 * n32 +
                n12 * n21 * n34 +
                n14 * n22 * n31 -
                n12 * n24 * n31
            ) +
            n44 * (
                n11 * n22 * n33 -
                n11 * n23 * n32 +
                n13 * n21 * n32 -
                n12 * n21 * n33 +
                n12 * n23 * n31 -
                n13 * n22 * n31
            )
        );
    }

    public getElement(index: number): number {
        return this.elements[index];
    }

    public setElement(index: number, value: number): void {
        this.elements[index] = value;
    }

    public getElements(): number[] {
        return [...this.elements];
    }
}

export class Quaternion {
    constructor(
        public x: number = 0,
        public y: number = 0,
        public z: number = 0,
        public w: number = 1
    ) {}

    public clone(): Quaternion {
        return new Quaternion(this.x, this.y, this.z, this.w);
    }

    public multiply(q: Quaternion): Quaternion {
        return new Quaternion(
            this.w * q.x + this.x * q.w + this.y * q.z - this.z * q.y,
            this.w * q.y - this.x * q.z + this.y * q.w + this.z * q.x,
            this.w * q.z + this.x * q.y - this.y * q.x + this.z * q.w,
            this.w * q.w - this.x * q.x - this.y * q.y - this.z * q.z
        );
    }

    public setFromAxisAngle(axis: Vec3, angle: number): Quaternion {
        const halfAngle = angle * 0.5;
        const s = Math.sin(halfAngle);
        
        this.x = axis.x * s;
        this.y = axis.y * s;
        this.z = axis.z * s;
        this.w = Math.cos(halfAngle);
        
        return this;
    }

    public normalize(): Quaternion {
        let len = Math.sqrt(
            this.x * this.x +
            this.y * this.y +
            this.z * this.z +
            this.w * this.w
        );

        if (len === 0) {
            this.x = 0;
            this.y = 0;
            this.z = 0;
            this.w = 1;
        } else {
            len = 1 / len;
            this.x *= len;
            this.y *= len;
            this.z *= len;
            this.w *= len;
        }

        return this;
    }

    public rotateVector(v: Vec3): Vec3 {
        const qVec = new Vec3(this.x, this.y, this.z);
        const uv = qVec.cross(v);
        const uuv = qVec.cross(uv);

        return v.add(uv.multiply(2 * this.w)).add(uuv.multiply(2));
    }

    public inverse(): Quaternion {
        const dot = 
            this.x * this.x +
            this.y * this.y +
            this.z * this.z +
            this.w * this.w;

        if (dot === 0) {
            return new Quaternion();
        }

        const invDot = 1.0 / dot;
        return new Quaternion(
            -this.x * invDot,
            -this.y * invDot,
            -this.z * invDot,
            this.w * invDot
        );
    }

    public slerp(q: Quaternion, t: number): Quaternion {
        if (t === 0) return this.clone();
        if (t === 1) return q.clone();

        const x = this.x;
        const y = this.y;
        const z = this.z;
        const w = this.w;

        let cosHalfTheta = w * q.w + x * q.x + y * q.y + z * q.z;

        if (cosHalfTheta < 0) {
            cosHalfTheta = -cosHalfTheta;
            q = new Quaternion(-q.x, -q.y, -q.z, -q.w);
        }

        if (cosHalfTheta >= 1.0) {
            return this.clone();
        }

        const halfTheta = Math.acos(cosHalfTheta);
        const sinHalfTheta = Math.sqrt(1.0 - cosHalfTheta * cosHalfTheta);

        if (Math.abs(sinHalfTheta) < 0.001) {
            return new Quaternion(
                x * 0.5 + q.x * 0.5,
                y * 0.5 + q.y * 0.5,
                z * 0.5 + q.z * 0.5,
                w * 0.5 + q.w * 0.5
            );
        }

        const ratioA = Math.sin((1 - t) * halfTheta) / sinHalfTheta;
        const ratioB = Math.sin(t * halfTheta) / sinHalfTheta;

        return new Quaternion(
            x * ratioA + q.x * ratioB,
            y * ratioA + q.y * ratioB,
            z * ratioA + q.z * ratioB,
            w * ratioA + q.w * ratioB
        );
    }
}
