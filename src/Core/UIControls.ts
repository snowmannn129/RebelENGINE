import { Panel } from './Panel';
import { Style } from './Style';

export interface UIControl {
    label: string;
    position: { x: number; y: number };
    size: { width: number; height: number };
    render(): void;
    getValue(): any;
    setValue(value: any): void;
    refresh(): void;
    getPosition(): { x: number; y: number };
    setPosition(x: number, y: number): void;
    getSize(): { width: number; height: number };
    setSize(width: number, height: number): void;
    getStyle(): Style | null;
    setStyle(style: Style): void;
}

export class Slider implements UIControl {
    public readonly label: string;
    public position: { x: number; y: number };
    public size: { width: number; height: number };
    private value: number;
    private min: number;
    private max: number;
    private onChange?: (value: number) => void;
    private style: Style | null = null;

    constructor(label: string, value: number, min: number, max: number, onChange?: (value: number) => void) {
        this.label = label;
        this.position = { x: 0, y: 0 };
        this.size = { width: 200, height: 20 }; // Default slider size
        this.value = Math.max(min, Math.min(max, value)); // Clamp initial value
        this.min = min;
        this.max = max;
        this.onChange = onChange;
    }

    public getStyle(): Style | null {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public render(): void {
        // Rendering logic will be implemented when we add the rendering system
    }

    public refresh(): void {
        this.render();
    }

    public getValue(): number {
        return this.value;
    }

    public setValue(value: number): void {
        this.value = Math.max(this.min, Math.min(this.max, value));
        if (this.onChange) {
            this.onChange(this.value);
        }
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }
}

export class Button implements UIControl {
    public readonly label: string;
    public position: { x: number; y: number };
    public size: { width: number; height: number };
    private callback: () => void;
    private style: Style | null = null;

    constructor(label: string, callback: () => void) {
        this.label = label;
        this.position = { x: 0, y: 0 };
        this.size = { width: 100, height: 30 }; // Default button size
        this.callback = callback;
    }

    public getStyle(): Style | null {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public render(): void {
        // Rendering logic will be implemented when we add the rendering system
    }

    public refresh(): void {
        this.render();
    }

    public getValue(): void {
        return undefined;
    }

    public setValue(value: any): void {
        // Buttons don't have a value to set
    }

    public click(): void {
        this.callback();
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }
}

export class Checkbox implements UIControl {
    public readonly label: string;
    public position: { x: number; y: number };
    public size: { width: number; height: number };
    private checked: boolean;
    private onChange?: (checked: boolean) => void;
    private style: Style | null = null;

    constructor(label: string, checked: boolean = false, onChange?: (checked: boolean) => void) {
        this.label = label;
        this.position = { x: 0, y: 0 };
        this.size = { width: 20, height: 20 }; // Default checkbox size
        this.checked = checked;
        this.onChange = onChange;
    }

    public getStyle(): Style | null {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public render(): void {
        // Rendering logic will be implemented when we add the rendering system
    }

    public refresh(): void {
        this.render();
    }

    public getValue(): boolean {
        return this.checked;
    }

    public setValue(value: boolean): void {
        this.checked = value;
        if (this.onChange) {
            this.onChange(this.checked);
        }
    }

    public toggle(): void {
        this.setValue(!this.checked);
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }
}

export class ColorPicker implements UIControl {
    public readonly label: string;
    public position: { x: number; y: number };
    public size: { width: number; height: number };
    private color: string;
    private onChange?: (color: string) => void;
    private style: Style | null = null;

    constructor(label: string, color: string = '#000000', onChange?: (color: string) => void) {
        this.label = label;
        this.position = { x: 0, y: 0 };
        this.size = { width: 150, height: 30 }; // Default color picker size
        this.color = color;
        this.onChange = onChange;
    }

    public getStyle(): Style | null {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public render(): void {
        // Rendering logic will be implemented when we add the rendering system
    }

    public refresh(): void {
        this.render();
    }

    public getValue(): string {
        return this.color;
    }

    public setValue(value: string): void {
        if (/^#[0-9A-Fa-f]{6}$/.test(value)) {
            this.color = value;
            if (this.onChange) {
                this.onChange(this.color);
            }
        }
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }
}

export class Dropdown implements UIControl {
    public readonly label: string;
    public position: { x: number; y: number };
    public size: { width: number; height: number };
    private options: string[];
    private selectedIndex: number;
    private onChange?: (selectedIndex: number, selectedValue: string) => void;
    private style: Style | null = null;

    constructor(label: string, options: string[], selectedIndex: number = 0, onChange?: (selectedIndex: number, selectedValue: string) => void) {
        this.label = label;
        this.position = { x: 0, y: 0 };
        this.size = { width: 150, height: 30 }; // Default dropdown size
        this.options = options;
        this.selectedIndex = Math.max(0, Math.min(options.length - 1, selectedIndex));
        this.onChange = onChange;
    }

    public getStyle(): Style | null {
        return this.style;
    }

    public setStyle(style: Style): void {
        this.style = style;
        this.refresh();
    }

    public render(): void {
        // Rendering logic will be implemented when we add the rendering system
    }

    public refresh(): void {
        this.render();
    }

    public getValue(): { index: number; value: string } {
        return {
            index: this.selectedIndex,
            value: this.options[this.selectedIndex]
        };
    }

    public setValue(index: number): void {
        if (index >= 0 && index < this.options.length) {
            this.selectedIndex = index;
            if (this.onChange) {
                this.onChange(this.selectedIndex, this.options[this.selectedIndex]);
            }
        }
    }

    public getPosition(): { x: number; y: number } {
        return this.position;
    }

    public setPosition(x: number, y: number): void {
        this.position = { x, y };
    }

    public getSize(): { width: number; height: number } {
        return this.size;
    }

    public setSize(width: number, height: number): void {
        this.size = { width, height };
    }

    public getOptions(): string[] {
        return [...this.options];
    }

    public setOptions(options: string[]): void {
        this.options = options;
        this.selectedIndex = Math.min(this.selectedIndex, options.length - 1);
        if (this.onChange && this.selectedIndex >= 0) {
            this.onChange(this.selectedIndex, this.options[this.selectedIndex]);
        }
    }
}
